/**
 * @file
 * @brief Code for handling DOQ tiles
 * @author Scott Morris
 * @author Alan Morris
 * @author Andrew Predoehl
 * @todo use of socket headers and functions violates libkjb design philosophy.
 *
 * Originally from TopoFusion.
 *
 * Functions and structures for downloading image and elevation data from
 *     terraserver or the jpl nasa server
 *
 * Functions "signed" with AMP are mine (Andy), all other code is the
 * Morrises' modulo some maintenance and adaptation that I've done.
 * This code requires Unix utility host(1) to be in the path.
 */
/*
 * $Id: download.cpp 15331 2013-09-16 08:33:56Z predoehl $
 */

#include "l/l_sys_debug.h"  /* For ASSERT. */
#include "l/l_sys_def.h"
#include "l/l_sys_std.h"
#include "l/l_sys_lib.h"
#include "l/l_sys_mal.h"
#include "l/l_sys_time.h"
#include "l/l_sys_tsig.h"
#include "l/l_error.h"
#include "l/l_global.h"
#include "l/l_debug.h"
#include "l_cpp/l_util.h"
#include "l_cpp/l_stdio_wrap.h"
#include "topo/master.h"
#include "topo_cpp/LatLong-UTMconversion.h"
#include "topo_cpp/download.h"

#include <iostream>

// the following includes and the socket calls should be wrapped into lib/l
extern "C" {
#include <sys/types.h>  /* for Socket data types */
#include <sys/socket.h> /* for socket(), connect(), send(), and recv() */
#include <netinet/in.h> /* for IP Socket data types */
#include <arpa/inet.h>  /* for sockaddr_in and inet_addr() */
#include <unistd.h>     /* for close() */
#include <netdb.h>      /* for gethostbyname */
}

#include <vector>
#include <string>
#include <sstream>

#define TRACE(x) KJB(TEST_PSE(x))   /**< TopoFusion macro for debug */

namespace
{

const int NO_SOCKET=-1;
/*int sockets[10];*/            // TopoFusion might have used ten, but . . .
int sockets[1] = { NO_SOCKET }; // I think one is enough, actually.

const int threadID = 0;         // Never seems to change, so let's const it
/*int socketTimeout = 60000;*/
bool usePersistentHTTP = true;

bool useProxy = false;
bool retry_delay = false;
std::string proxy = "";
int proxyPort = 10;
int failCount = 0;

const bool VERBOSE = false;

/*
 * Error check the results, and this is where it starts to get weird.
 * We expect two IP addresses, see below.  If we get just one but we
 * recognize it, I guess that is OK (not yet observed in the wild).
 * If we get a strange number of address, or if we get never-before-seen
 * addresses then we just wig out and fail.  Call it confirmation bias
 * taken to an extreme.  This function ultimately does not look up
 * addresses to discover them, but to confirm them.  Someday it won't
 * work anymore like that.
 * AMP
 */
int terraserver_validate(
    std::vector< std::string >* ips,
    const std::string& hostname
)
{
    KJB(NRE( ips ));

#if 0
    // we already know what the answers are supposed to be.  we think.
    const std::string   the_right_answer = "131.107.151.135",
                        almost = the_right_answer.substr( 0, 14 ) + "6";

    if ( 1 == ips -> size() && the_right_answer == ips -> at( 0 ) )
    {
        KJB(TEST_PSE(( "Warning: DNS says %s has but one address.\n", 
                       hostname.c_str())));
        return kjb_c::NO_ERROR;
    }

    if ( ips -> size() != 2 )
    {
        KJB(set_error( "host(1) gave %u addresses for %s, expected 2.\n",
                       ips -> size(), hostname.c_str()));
        return kjb_c::ERROR;
    }

    if ( ips -> at( 0 ) == ips -> at( 1 ) )
    {
        KJB(set_error( "host(1) gave duplicate addrs (%s) for %s",
                       ips -> at( 0 ).c_str(), hostname.c_str()));
        return kjb_c::ERROR;
    }

    if ( ips -> at( 0 ) > ips -> at( 1 ) )
    {
        std::swap( ips -> at( 0 ), ips -> at( 1 ) );
    }

    if ( the_right_answer != ips -> at( 0 ) || almost != ips -> at( 1 ) )
    {
        KJB(set_error( "host(1) gave unexpected/unusable results for %s",
                       hostname.c_str()));
        KJB(TEST_PSE(( "DNS lookup on %s gave unexpected results.\n"
                "(Addresses were %s and %s.)  We assume that is wrong.\n"
                "If they really have moved their server, someone will\n"
                "have to update lib/topo_cpp/%s.\n", hostname.c_str(),
                ips -> at( 0 ).c_str(), ips -> at( 1 ).c_str(),
                __FILE__ )));
        return kjb_c::ERROR;
    }
#else
    const std::string the_right_answer("65.54.113.33");
    if (ips->size() < 1 || ips->front() != the_right_answer)
    {
        KJB(set_error(
            "Bad or unusable results when trying to look up msrmaps.com." ));
        for (size_t i = 0; i < ips -> size(); ++i)
        {
            KJB(add_error( "Result # %d from host(1) was %s.\n",
                           ips -> at(i).c_str()));
        }
        KJB(add_error("Expected address was %s.\n", the_right_answer.c_str()));
        KJB(add_error("If the address really has changed, someone will have "
                    "to update lib/topo_cpp, either to insert the new address "
                    "or to skip this test as unnecessary paranoia."));
        KJB(NOTE_ERROR());
        return kjb_c::ERROR;
    }
#endif

    return kjb_c::NO_ERROR;
}


// perform DNS lookup by shelling out to host(1).
// AMP
int really_look_up_terraserver_address( std::string* quads )
{
    using namespace kjb_c;
    NRE( quads );

    std::vector< std::string > adrs;
    kjb::Temporary_File tf;
    const std::string   dns = "msrmaps.com",
                        host = "host " + dns + " > " + tf.get_filename();
    kjb_system( host.c_str() );             // shell out
    ERE( kjb_fflush( tf ) );
    ERE( kjb_fseek( tf, 0L, SEEK_SET ) );   // rewind

    // Scan every line of output
    std::string line;
    for( int rc; NO_ERROR == ( rc = kjb::getline( tf, &line ) ); line.clear() )
    {
        // Scan a line
        const std::string fmt = dns + " has address %d.%d.%d.%d";
        int q[ 4 ], ct = sscanf( line.c_str(), fmt.c_str(), q, q+1, q+2, q+3 );
        if ( ct != 4 )
        {
            set_error( "Unable to parse %s address", dns.c_str() );
            return ERROR;
        }

        // Convert numbers in dotted quad back to string (a bit clunky)
        std::ostringstream iss;
        for( int iii = 0; iii < 4; ++iii )
        {
            if ( q[ iii ] < 0 || 255 < q[ iii ] )
            {
                set_error( "Bad quad value %d detected", q[ iii ] );
                return ERROR;
            }
            iss << q[ iii ];
            if ( iii < 3 ) iss << '.';
        }
        adrs.push_back( iss.str() );
        ASSERT( 6 < adrs.at( adrs.size() - 1 ).size() );

        // paranoia
        if ( adrs.size() > 2 ) break;
    }

    ERE( terraserver_validate( &adrs, dns ) );
    *quads = adrs[ 0 ];

    return NO_ERROR;
}



// because of the weird behavior at msrmaps.com this function behaves oddly too
// AMP
int get_terraserver_address( std::string* dotted_quads )
{
    KJB( NRE( dotted_quads ) );

    // cache the value after the first lookup
    static std::string quads;

    // look up IP address of msrmaps.com using host(1)
    if ( 0 == quads.size() )
    {
        KJB( ERE( really_look_up_terraserver_address( &quads ) ) );
        ASSERT( 0 < quads.size() );
    }

    *dotted_quads = quads;
    return kjb_c::NO_ERROR;
}


// AMP
void debug_print( const char* msg, const kjb::TopoFusion::tile_entry* te )
{
    ASSERT( te );
    TRACE(( "DEBUG: %s tile_entry(%d,%d,%d,%d)\n", msg,
                te->x, te->y, (int)te->tileset, (int)te->zone ));
}


int findContentLength(char *buffer, int /*size*/ )
{
    char cstring[] = "Content-Length: ";
    char *start;
    int contentLength = -1;

    start = strstr(buffer,cstring);
    if (start != NULL)
    {
        // check that we have all the way up to the newline in: Content-Length: 2343\n
        char *p = start;
        while (p[0] != '\n' && p[0] != '\0')
        {
            p++;
        }
        if (p[0] == '\0') return -1;
        start+=16; //skip the string
        sscanf(start,"%d",&contentLength);
        return contentLength;
    }
    return -1;
}


int handleChunked(char *buffer,int bufsize)
{
    using namespace kjb_c;

#ifdef debugnasa
    TRACE(( "handling chunked data, bufsize = %d\n",bufsize));
    TRACE(( "writing out response.txt\n"));

    FILE *f = kjb_fopen("reponse.txt","wb");

    kjb_fwrite(buffer,1,bufsize,f);
    kjb_fclose(f);
#endif

    char *outbuffer = (char *) KJB_MALLOC(sizeof(char)*bufsize);

    if (outbuffer == NULL) return -4;
    /*
    char buf[50000];
            = (char *)malloc(sizeof(char)*bufsize);  // temp buffer
     */

    char *curpos = strstr(buffer,"chunked");

    if (curpos == NULL)
    { // no chunked in buffer
        kjb_free( outbuffer );
        curpos = strstr(buffer,"jpeg");
        if (curpos == NULL) curpos = strstr(buffer,"png");

        if (curpos == NULL) return -3;
        return bufsize; // still a png, but not chunked
    }
    char *temp = strstr(buffer,"jpeg");

    if (temp!=NULL) TRACE(( "Got a Jpeg! what?!?\n"));

#ifdef debugnasa
    TRACE(( "found chunked in data\n"));
#endif

#ifdef debugnasa
    TRACE(( "curpos = %d\n",curpos));
#endif

    // skip to blank line, then go past it.  Either \n\n or \r\n\r\n

    if ((curpos = strstr(buffer,"\n\n")) == NULL)
    {
        if ((curpos = strstr(buffer,"\r\n\r\n")) == NULL)
        { // read chunked line
            return -1;
        }
        curpos+=4;
    }
    else
    {
        curpos+=2;
    }

#ifdef debugnasa
    TRACE(( "curpos = %d\n",curpos));
#endif

    bool done = false;
    int outpos = 80;  // start with enough room that the "BEGIN TILE" stuff can be written at the start of the tile
    int chunksize;

    while (!done)
    {
        unsigned uchunksize;
        if (sscanf(curpos,"%x",&uchunksize) != 1) return -5; // read size line
        ASSERT( uchunksize < INT_MAX );
        chunksize = static_cast<int>( uchunksize );
#ifdef debugnasa
        TRACE(( "chunksize = %d\n",chunksize));
#endif

        if (chunksize <= 0)
        {
            done = true;
            break;
        }

        if ((curpos = strstr(curpos,"\n")) == NULL)
        {
            return -6;  // move pointer to start of data
        }

#ifdef debugnasa
        TRACE(( "moved past chunked line, outpos = %d\n",outpos));
#endif

        curpos++;

        kjb_memcpy(&outbuffer[outpos], curpos, chunksize);
        outpos+=chunksize;

        curpos+=chunksize;

        if ((curpos = strstr(curpos,"\n")) == NULL)
        {
            return -7;  // move pointer to start of data
        }

        curpos++;  // skip past \r\n to next 'sizeof' chunked line
    }

#ifdef debugnasa
    TRACE(( "processed chunks\n"));
#endif

    kjb_memcpy(buffer,outbuffer,outpos);

    kjb_free(outbuffer);

    return outpos;
}

int checkTile (char *buffer, int bufsize)
{
    //TRACE(( "Checking Tile\n"));
    int i=0;
    bool jpegfound = false;
    while ((i < (bufsize-4)) && (jpegfound == false))
    {
        if (            buffer[i  ] == 'J')
        {
            if (        buffer[i+1] == 'F'
                    &&  buffer[i+2] == 'I'
                    &&  buffer[i+3] == 'F')
            {
                jpegfound = true;
            }
        }

        if (            buffer[i  ] == 'G')
        {
            if (        buffer[i+1] == 'I'
                    &&  buffer[i+2] == 'F'
                    &&  buffer[i+3] == '8')
            {
                jpegfound = true;
            }
        }
        i++;
    }

    if (jpegfound == false)
    {
#if 0
        using namespace kjb_c;
        DbgLog("checkTile: Didn't find JFIF header, first 512 bytes:\n");
        char buf[511];
        size_t cplen = std::min(bufsize, sizeof(buf)-1);
        kjb_memcpy (buf, buffer, cplen);
        buf[ cplen ] = 0;
        DbgLog("%s\n",buf);
        DbgLog("No tile from TerraServer\n");
#endif
        return -2;
    }

    return bufsize;
}



// AMP
void debug_print( const hostent* hhh )
{
    ASSERT( hhh );
    TRACE(( "Name of host: %s\n", hhh->h_name ));
    TRACE(( "DEBUG: List of aliases:" ));
    for( char **p=hhh->h_aliases; *p; )
    {
        TRACE(( " %s", *p++ ));
    }
    TRACE(( " (END-OF-LIST)\n" ));
    int hat = hhh->h_addrtype;
    ASSERT( AF_INET==hat || AF_INET6==hat );
    TRACE(( "Host address type: %s\n",
                                ( AF_INET==hat ? "AF_INET" : "AF_INET6" ) ));
    TRACE(( "Length of address: %d\n", hhh->h_length ));
}


// This performs the get.  Return value is nonnegative (number of bytes got)
// iff successful.  So, a negative return value means failure.
int GetHTTP(
    const std::string& lpServerName,
    int port,
    const std::string& lpRequest,
    char *buffer,
    int bufsize
)
{
    struct in_addr iaHost;
    struct hostent *lpHostEntry=0;

    if (0 == buffer)
    {
        KJB(NPE(buffer));
        return -1;
    }

    // Use inet_aton() to determine if we're dealing with a name or an address
    int success=inet_aton( lpServerName.c_str(), & iaHost );
    if ( success==0 )
    {
        // Wasn't an IP address string, assume it is a name
        // TODO:  gethostbyname is deprecated; should use getaddrinfo
        lpHostEntry = gethostbyname( lpServerName.c_str() );
    }
    else
    {
        // It was a valid IP address string
        lpHostEntry = gethostbyaddr((const char *)&iaHost,
                                            sizeof(struct in_addr), AF_INET);
    }

    //printf("hostaddr = %d\n",lpHostEntry->h_length);
    if (lpHostEntry == NULL)
    {
        TRACE(( "%s(#%d): Error using gethostbyname()\n", __func__, threadID));
        return -1;
    }
    //debug_print( lpHostEntry );

    if ( lpServerName == "wms.jpl.nasa.gov" && sockets[threadID] != NO_SOCKET )
    {
        /*
         * close any open socket since we know the nasa server doesn't allow
         * persistent connections.
         */
        close(sockets[threadID]);
        sockets[threadID] = NO_SOCKET;
    }

    if (sockets[threadID] == NO_SOCKET)
    {

        /*DbgLog("%s(#%d): Connecting socket\n", __func__, threadID);
          TRACE(( "%s(#%d): Creating Socket\n", __func__, threadID));*/

        sockets[threadID] = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (sockets[threadID] == NO_SOCKET)
        {
            TRACE(( "%s(#%d): Error creating socket\n", __func__, threadID));
            return -1;
        }

        std::string tsad;
        if ( kjb_c::ERROR == get_terraserver_address( &tsad ) )
        {
            kjb_c::set_error( "Unable to obtain terraserver's IP address" );
            return -1;
        }

        //TRACE(("%s(#%d): Connecting to TerraServer\n", __func__, threadID));
        sockaddr_in saServer;
        saServer.sin_port = htons(port);
        //debug_print("Accessing terraserver on port", port );

        // Fill in the rest of the server address structure
        saServer.sin_family = AF_INET;
        saServer.sin_addr.s_addr = inet_addr( tsad.c_str() );

        //TRACE(("%s(#%d): Connecting Socket\n", __func__, threadID));
        /*
        fprintf( stderr, "ret = %d,hostaddr = %d,port = %d, "
            "saServer.sin_addr = %d\n", ret, lpHostEntry->h_addr_list[0],
            port, saServer.sin_addr);
        */

        //printf("socket = %d\n",sockets[threadID]);

        // Connect the socket
        int nRet = connect(sockets[threadID],
                            (struct sockaddr *)&saServer, sizeof(saServer));
        if (nRet == -1)
        {
            TRACE(( "%s(#%d): Error in connect(), error=%d\n", __func__,
                                                            threadID, errno));
            close(sockets[threadID]);
            sockets[threadID] = NO_SOCKET;
            return -1;
        }
    }

    /*
    TRACE(("%d: %s(#%d): Sending Request\n",GetTickCount(),__func__,threadID));
    */

    int nRet = send(sockets[threadID], lpRequest.c_str(), lpRequest.size(), 0);
    if ( nRet != int( lpRequest.size() ) )
    {
        /*
        TRACE(("%s(#%d): Error in send(), error=%d\n",__func__,threadID,nRet));
        */
        int en = errno;
        TRACE(("%s: send() failed %u %d\n", __func__, lpRequest.size(), nRet));
        TRACE(("%s\n", strerror(en) ));

        close(sockets[threadID]);
        sockets[threadID] = NO_SOCKET;
        return -1;
    }

    int bytesReceived = 0;

    while(1)
    {
        int contentLength = -1;
        int headposition = -1;

        // Wait to receive, nRet = NumberOfBytesReceived
        nRet = recv(sockets[threadID], buffer+bytesReceived,
                                                    bufsize-bytesReceived, 0);
        /*
        DbgLog("%d: %s(#%d): recv() returned %d, (timeout=%d)\n",
            GetTickCount(), __func__, threadID,nRet,socketTimeout);
        */

        if (nRet == -1)
        {
            close(sockets[threadID]);
            sockets[threadID] = NO_SOCKET;
            return -1;
        }

        bytesReceived+=nRet;
        buffer[bytesReceived] = 0;

        if (contentLength == -1)
        {
            char *headEnd = strstr(buffer,"\r\n\r\n");
            if (headEnd != NULL)
            {
                //header has come through
                contentLength = findContentLength(buffer,bytesReceived);
                headposition = headEnd - buffer + 2;
            }
        }


        //TRACE(( "recv() returned %d bytes\n", nRet));

        // Did the server close the connection?
        if (nRet == 0)
        {
            TRACE(( "Server closed connection, closing socket\n"));
            close(sockets[threadID]);
            sockets[threadID] = NO_SOCKET;
            break;
        }

        if (contentLength != -1)
        {
            if (bytesReceived - headposition >= contentLength) break;
        }

        if (contentLength == 0)
        {
            TRACE(( "%s(#%d): Bad Content Length (=0)\n", __func__, threadID));
            return -2;
        }
    }


    if (usePersistentHTTP == false && sockets[threadID] != NO_SOCKET)
    {
        TRACE(( "closing socket (not persistent)\n"));
        close(sockets[threadID]);
        sockets[threadID] = NO_SOCKET;
    }

    //TRACE(("%s(#%d): Received %d bytes!\n", _func__,threadID,bytesReceived));

    if (bytesReceived == 0)
    {
        TRACE(("%s(#%d): Received 0 bytes!\n", __func__, threadID));
    }

    //TRACE(( "%s(#%d): Finished\n", __func__, threadID));
    return bytesReceived;
}


} // end anonymous namespace


namespace kjb
{
namespace TopoFusion
{


int download_tile( const tile_entry *entry, char *buffer, int bufsize )
{
    using kjb_c::TopoFusion::TileSource;

    if (VERBOSE) 
    {
        std::cout << __func__ << ' ' << entry -> x << ' '
            << entry -> y << ' ' << int(entry -> tileset) << ' '
            << entry -> zone << '\n';
    }

    bool useNasa =  entry->tileset >= kjb_c::TopoFusion::NASA_MIN
                    &&  entry->tileset <= kjb_c::TopoFusion::NASA_MAX;
    if (useNasa)
    {
        if (retry_delay)
        {
            if (kjb_c::get_real_time() < 10000) // less than ten secs elapsed?
            {
                // this is the duration scott specified -- excessive?
                kjb_c::nap(1000000); // one million milliseconds = 17 minutes
                return -4;
            }
            retry_delay = false;
        }
        TRACE(( "Timeout over, trying again\n"));
    }

    const std::string server( useNasa ?  "wms.jpl.nasa.gov" : "msrmaps.com" );

    std::string prefix( "GET " );
    if (useProxy) prefix += "http://" + server;
    prefix += "/";

    std::string suffix( "Connection: Close\r\n\r\n" );

    // build tile request string, which varies depending on nasa or terraserver
    char tilestr[255];
    if (useNasa)
    {
        const kjb_c::TopoFusion::st_TileSource& meta_tile
                                    = TileSource[ int( entry -> tileset ) ];
        const long &UtmSize = meta_tile.UTM_Size;
        const double &metersPerPixel = meta_tile.metersPerPixel;

        pt  u1 = make_pt(   entry -> x * UtmSize + 23 - 2 * metersPerPixel,
                            entry -> y * UtmSize + 158 - 2 * metersPerPixel,
                            entry -> zone ),
            u2 = make_pt(   u1.x + UtmSize + 4 * metersPerPixel,
                            u1.y + UtmSize + 4 * metersPerPixel,
                            entry -> zone );

        double lat, lon, lat2, lon2;
        kjb::TopoFusion::utm_to_lat_long( 23, u1, lat, lon );
        kjb::TopoFusion::utm_to_lat_long( 23, u2, lat2, lon2 );

#if 0
        sprintf(request,"GET /landsat.cgi?zoom=0.0011112&x0=%f&y0=%f"
            "&action=zoom0.0002778&layer=modis%252Cglobal_mosaic"
            "&pwidth=200&pheight=200",

        sprintf(request,"GET /wms.cgi?wmtver=0.9&request=GetMap"
            "&format=image/jpeg&bbox=%f,%f,%f,%f&width=200&height=200"
            "&layers=global_mosaic&styles=visual&srs=EPSG%%3A4326 "
            "HTTP/1.1\r\nHost: wms.jpl.nasa.gov\r\n"
            "Connection: Keep-Alive\r\n\r\n",
            lon,lat,lon2,lat2);

        sprintf(request,"GET /wms.cgi?wmtver=0.9&request=GetMap"
            "&format=image/jpeg&bbox=%f,%f,%f,%f&width=200&height=200"
            "&layers=modis%%2Cglobal_mosaic&styles=&srs=EPSG%%3A4326 "
            "HTTP/1.1\r\nHost: wms.jpl.nasa.gov\r\n"
            "Connection: Keep-Alive\r\n\r\n",
            lon,lat,lon2,lat2);
#endif

        kjb_c::kjb_sprintf( tilestr, sizeof(tilestr), "wms.cgi?request=GetMap"
                "&layers=global_mosaic&srs=EPSG:4326&width=204&height=204"
                "&bbox=%f,%f,%f,%f"
                "&format=image/jpeg&styles=visual&zoom= "
                "HTTP/1.1\r\nHost: wms.jpl.nasa.gov\r\n",
                lon, lat, lon2, lat2
            );
    }
    else
    {
        kjb_c::kjb_sprintf( tilestr, sizeof(tilestr),
                    "tile.ashx?S=%d&T=%d&X=%d&Y=%d&Z=%d "
                    /*"HTTP/1.1\r\nHost: terraserver-usa.com\r\n", obsolete */
                    "HTTP/1.1\r\nHost: msrmaps.com\r\n",
                    TileSource[ int(entry->tileset) ].s_val,
                    TileSource[ int(entry->tileset) ].t_val,
                    entry->x,
                    entry->y,
                    entry->zone
                );

        if (usePersistentHTTP) suffix = "Connection: Keep-Alive\r\n\r\n";
    }

    // perform GET
    const int port = useProxy ? proxyPort : 80;
    const std::string   &host( useProxy ? proxy : server ),
                        request( prefix + tilestr + suffix );
    int retval = GetHTTP( host, port, request, buffer, bufsize );

    // process response to GET
    if (retval == -1)
    {
        //Sleep(2000);
        failCount++;
        TRACE(("download_tile: failcount = %d\n", failCount));
    }
    else
    {
        failCount = 0;
        //TRACE(("download_tile: failcount = %d\n",failCount));
        //retval = checkMoved(threadID,buffer,retval,bufsize);
        if (retval == -1)
        {
            failCount++;
            TRACE(("download_tile: failcount = %d\n", failCount));
        }
        else
        {
            //printf("checking tile.. ");
            retval = checkTile(buffer, retval);
        }
        if (useNasa)
        {
            if (retval > 0) retval = handleChunked(buffer,retval);

            if (retval <= 0)
            {   // probably overloading it with too many requests, this seems
                // to be necessary to avoid complete shut out of downloading
                kjb_c::init_real_time();
                retry_delay = true;
                //Sleep(5000);
                TRACE(( "Dying, waiting for timeout\n"));
            }
        }
    }

    return retval;
}

} // end namespace TopoFusion
} // end namespace kjb

