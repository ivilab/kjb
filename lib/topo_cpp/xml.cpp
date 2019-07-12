/**
 * @file
 * @brief Implementation of GPX format input and output
 * @author Scott Morris
 * @author Alan Morris
 * @author Andrew Predoehl
 * @warning This is SO not threadsafe.
 *
 * Originally from TopoFusion.
 *
 * I (Andrew) made many superficial changes to port it into the KJB library.
 * Also I added the table of leap seconds, and the policy of using them.
 */

/*
 * $Id: xml.cpp 21596 2017-07-30 23:33:36Z kobus $
 */

#include "l/l_sys_debug.h"  /* For ASSERT. */
#include "l/l_sys_lib.h"
#include "l/l_sys_mal.h"
#include "l/l_sys_io.h"
#include "l/l_error.h"
#include "l/l_debug.h"
#include "l/l_global.h"
#include "l/l_string.h"
#include "l_cpp/l_util.h"
#include "l_cpp/l_exception.h"
#include "l_cpp/l_stdio_wrap.h"

#include <sstream>

// C++ definitions from TopoFusion
#include <topo_cpp/LatLong-UTMconversion.h>
#include <topo_cpp/xml.h>

// Precision of 1e-6 degrees (6 inches) is the limit of the original GPS data
#define LATF    "lat=\"%.7f\""   /**< printf-style format for latitude data */
#define LONF    "lon=\"%.6f\""   /**< printf-style format for longitude data */
#define LATLONF LATF " " LONF    /**< printf-style format for lat,lon data */

/* Kobus: Changed this from test_pso, to pso, as test_pso has changed. It is not
 * clear what exact behaviour is wanted. 
*/
#define DbgLog kjb_c::pso   /**< TopoFusion macro for debug messages */

namespace
{

const char* GPX_OUTPUT_FORMAT_VERSION = "2.18";

const int FULL_TAG_SIZE=65536;
const int MAX_ATTRIBS=10;

/* Whether a given year is a leap year. */
inline int ISLEAP( int year )
{
    return ((year % 4) == 0 && ((year % 100) != 0 || (year % 400) == 0)) ? 1:0;
}



struct countStruct_type
{
    int numWaypoints;
    int numTrksegs[2000];
    int numTracks;
    int numPoints[2000];
    void db_print() const;
} countStruct;

typedef struct
{
    char name[FULL_TAG_SIZE+1];
    char fulltag[FULL_TAG_SIZE+1];
    int numAttrib;
    char attributes[MAX_ATTRIBS][FULL_TAG_SIZE+1];
    char values[MAX_ATTRIBS][FULL_TAG_SIZE+1];
    bool selfEnding; //whether it was like <trkpt lat=54.43 lon=543 /> or just <trkpt lat=54.43 lon=543>
                     //used so we know whether or not to look for </trkpt>
} tag;


char *xmlFile=0;
long xmlPosition, xmlSize;

tag theTag;
char theString[FULL_TAG_SIZE+1];

inline
bool is_tag_name( const char* string )
{
    using kjb_c::kjb_strcmp;
    return STRCMP_EQ( theTag.name, string );
}

/// copy a short stretch of file into a character array buffer.
#define FILE_PEEK( buf )                                            \
    do                                                              \
    {                                                               \
        using namespace kjb_c;                                      \
        ASSERT( xmlFile );                                          \
        ASSERT( xmlPosition < xmlSize );                            \
        kjb_strncpy( (buf), xmlFile + xmlPosition, sizeof(buf) );   \
    } while( 0 )


/* The number of days in each month. */
const int MONTHDAYS[] =
{
    31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31
};

/*
 * UTC includes leap seconds.  Here is a table of leap seconds, derived from
 * the following source:
 * http://hpiers.obspm.fr/eop-pc/index.php?index=TAI-UTC_tab&lang=en
 *
 * Feel free to augment this list as new leap seconds are announced, but make
 * sure to add new entries in strictly increasing order, except for the
 * sentinel value at the end of list (which must remain).
 * 
 * added by AMP
 */
const time_t LEAP_SECONDS[] =
{
                // First value is number of UTC seconds between 1 Jan. 1970 and
    78796801,   // 1 Jul. 1972
                // In other words, the last minute of 30 June 1972 lasted 61 s.

                // Second value is number of UTC seconds btw. 1 Jan. 1970 and
    94694402,   // 1 Jan. 1973

                // And so on, likewise, for
    126230403,  // 1 Jan. 1974
    157766404,  // 1 Jan. 1975
    189302405,  // 1 Jan. 1976
    220924806,  // 1 Jan. 1977
    252460807,  // 1 Jan. 1978
    283996808,  // 1 Jan. 1979
    315532809,  // 1 Jan. 1980
    362793610,  // 1 Jul. 1981
    394329611,  // 1 Jul. 1982
    425865612,  // 1 Jul. 1983
    489024013,  // 1 Jul. 1985
    567993614,  // 1 Jan. 1988
    631152015,  // 1 Jan. 1990
    662688016,  // 1 Jan. 1991
    709948817,  // 1 Jul. 1992
    741484818,  // 1 Jul. 1993
    773020819,  // 1 Jul. 1994
    820454420,  // 1 Jan. 1996
    867715221,  // 1 Jul. 1997
    915148822,  // 1 Jan. 1999
    1136073623, // 1 Jan. 2006
    1230768024, // 1 Jan. 2009
    1341100825, // 1 Jul. 2012
    0           // sentinel: end of list
};

void countStruct_type::db_print() const
{
    DbgLog( "Num waypoints: %d\nNum tracks: %d\n",
            numWaypoints, numTracks );
    if ( 1 == numTracks ) DbgLog( "Track size: %d\n", numPoints[0]);

    if ( 1 < numTracks )
    {
        DbgLog( "\tTrack sizes:" );
        for( int iii = 0; iii < numTracks; ++iii )
            DbgLog( "\t%d", numPoints[ iii ] );
        DbgLog( "\n" );
    }
}


int XML_ParseTag ()
{
    int len = kjb_c::signed_strlen(theTag.fulltag);

    //int pos=0;
    int i=0;

    while( i<len && theTag.fulltag[i] != ' ' )
    {
        ++i; // skip the name (<trkpt, <bounds, etc)
    }

    i++;
    int attribPosition = i;


    //int numAttrib = 0;
    while (i<len)
    {
        // skip whitespace
        while (((theTag.fulltag[i] == ' ') || (theTag.fulltag[i] == '\r') ||
            (theTag.fulltag[i] == '\n')) && i<len)
        {
            i++;
        }

        if (i>=len) return -1;

        if (        theTag.fulltag[i] == '?'
                ||  theTag.fulltag[i] == '/'
                ||  theTag.fulltag[i] == '>')
        {
            i = len; // exit loop
            continue;
        }

        while (theTag.fulltag[i] != '=')
        {
            i++;
            if (i>=len) return -1;
        }
        theTag.numAttrib++;
        i++; // skip '='
        if (theTag.fulltag[i] != '"') return -1;
        i++; // skip '"'

        while (theTag.fulltag[i] != '"')
        {
            i++;
            if (i>=len) return -1;
        }
        i++; // skip '"'
    }

    if (theTag.numAttrib >= MAX_ATTRIBS) return -1;

    if (theTag.numAttrib >= 1)
    {
        int idx=0;
        int startPos, endPos;

        i = attribPosition;
        while (i<len)
        {
            // skip whitespace
            while (     theTag.fulltag[i] == ' '
                    ||  theTag.fulltag[i] == '\r'
                    ||  theTag.fulltag[i] == '\n')
            {
                i++;
            }

            if (        theTag.fulltag[i] == '?'
                    ||  theTag.fulltag[i] == '/'
                    ||  theTag.fulltag[i] == '>')
            {
                i = len;
                continue;
            }

            startPos = i;
            while (theTag.fulltag[i] != '=') i++;
            endPos = i;
            kjb_c::kjb_strncpy ( theTag.attributes[idx],
                                theTag.fulltag+startPos, endPos-startPos+1 );
            theTag.attributes[idx][endPos-startPos] = 0;

            i++; // skip '='
            if (theTag.fulltag[i] != '"') return -1;
            i++; // skip '"'

            startPos = i;
            while (theTag.fulltag[i] != '"') i++;
            endPos = i;
            kjb_c::kjb_strncpy (theTag.values[idx], theTag.fulltag+startPos,
                                                            endPos-startPos+1);
            theTag.values[idx][endPos-startPos] = 0;
            idx++;

            i++; // skip '"'
        }
    }

    return 0;
}


bool XML_GetNextTag ()
{
    ASSERT( xmlFile );
    char c;
    //int numRead;

    theTag.selfEnding = false;

    while (1)
    {
        c = xmlFile[xmlPosition++];
        if (c == 0 || xmlPosition > xmlSize) return false;

        if (c == '<')
        {//start of tag
            int pos=0;
            bool nameDone = false;
            while ((c != '>') && (pos < FULL_TAG_SIZE))
            {
                if ((c == ' ') && (!nameDone))
                {
                    theTag.name[pos-1] = 0;
                    nameDone = true;
                }

                if ((!nameDone) && (c != '<')) theTag.name[pos-1] = c;
                theTag.fulltag[pos++] = c;
                c = xmlFile[xmlPosition++];
                if (c == 0 || xmlPosition > xmlSize) return false;
            }

            // detect self-terminating tags
            if (xmlFile[xmlPosition-1] == '>' && xmlFile[xmlPosition-2] == '/')
            {
                theTag.selfEnding = true;
            }

            theTag.fulltag[pos] = 0;
            theTag.numAttrib = 0;
            if (!nameDone)
            {
                theTag.name[pos-1] = 0;
            }

            return true;
        }
    }
    return false;
}


double XML_GetAttrib_Double(const char *attrib)
{
    for (int i=0;i<theTag.numAttrib;i++)
    {
        using kjb_c::kjb_strcmp;
        if ( STRCMP_EQ( theTag.attributes[i], attrib ) )
        {
            return atof(theTag.values[i]);
        }
    }

    return -1;
}

double XML_Read_Double()
{
    double d;

    char buf[100];
    FILE_PEEK( buf );

    int n, rc = sscanf( buf, "%lf%n", &d, &n );
    if (rc == 2) xmlPosition+=n;

    return d;
}

long XML_Read_Long()
{
    ASSERT( xmlFile );

    char buf[100];
    FILE_PEEK( buf );

    long l;
    int n, rc = sscanf( buf, "%ld%n", &l, &n);
    if (rc == 2) xmlPosition+=n;

    return l;
}


void XML_Write_String(FILE *f, const char *s)
{
    using namespace kjb_c;

    if (!s) return;

    bool useCdata = false;

    if (FIND_CHAR_YES(s, '<') || FIND_CHAR_YES(s, '&'))
    {
        if (FIND_STRING_NO(s, "]]>"))
        {
            useCdata = true;
        }
        // else, the CDATA syntax is unavailable to us.
    }
    // else, the input is so simple that it is safe to print directly.

    if (useCdata)
    {
        kjb_c::kjb_fprintf (f, "<![CDATA[%s]]>", s);
    }
    else
    {
        std::ostringstream str;
        for( ; *s; ++s )
        {
            switch( *s )
            {
                case '<':   str << "&lt;";  break;
                case '>':   str << "&gt;";  break;
                case '&':   str << "&amp;"; break;
                default:    str << *s;      break;
            }
        }
        kjb_c::kjb_fputs( f, str.str().c_str() );
    }
}


void XML_Write_StringTag(FILE *f, const char *s, const char *tag)
{
    if ((s) && (*s))
    {
        kjb_c::kjb_fprintf (f," <%s>",tag);
        XML_Write_String(f,s);
        kjb_c::kjb_fprintf (f,"</%s>\n",tag);
    }
}


// AMP (the Morris version was a trainwreck)
int substitute(char *s, const std::string& look, char repl)
{
    ASSERT( 1 < look.size() );
    ASSERT( repl != 0 );

    std::string r( s );
    const size_t SZ0 = r.size();
    int count = 0; // number of substitutions

    for( size_t ix = 0; (ix = r.find( look, ix )) < SZ0; )
    {
        ++count;
        r.replace( ix, look.size(), 1, repl );
    }

    ASSERT( r.size() <= SZ0 );
    if ( count ) std::copy( r.begin(), r.end(), s );
    return count;
}








bool XML_Read_String()
{
    char c;
    int pos = 0;
    char *nonCdata = theString;

    ASSERT( xmlFile );

    while (1)
    {
        // read a character
        c = xmlFile[xmlPosition++];
        if (c == 0 || xmlPosition > xmlSize) return false;

        if (c != '<')
        {
            if (pos < FULL_TAG_SIZE) theString[pos++] = c;
        }
        else    // If it's '<', it's either the end of the string
        {       // or the start of a CDATA section.
            using kjb_c::kjb_strcmp;

            // first perform subsitutions on this last non-cdata section
            theString[pos] = 0;
            pos = pos - (3*substitute(nonCdata,"&gt;",'>'));
            pos = pos - (3*substitute(nonCdata,"&lt;",'<'));
            pos = pos - (4*substitute(nonCdata,"&amp;",'&'));
            pos = pos - (5*substitute(nonCdata,"&apos;",'\''));
            pos = pos - (5*substitute(nonCdata,"&quot;",'"'));

            // read 7 characters to look for "![CDATA"


            char cdata[] = "![CDATA";   // just big enough to hold exactly this
            ASSERT( 8u == sizeof( cdata ) );
            if ( xmlPosition + long(sizeof( cdata )) > xmlSize ) return false;
            kjb_c::kjb_strncpy( cdata, xmlFile+xmlPosition, sizeof(cdata) );
            xmlPosition += sizeof(cdata)-1;

            // if CDATA
            if ( STRCMP_EQ( cdata, "![CDATA" ) )
            {
                c = xmlFile[xmlPosition++];
                if (c == 0 || xmlPosition > xmlSize) return false;

                //numRead = fread(&c,1,1,f); if (numRead != 1) return false; // skip '['

                bool cdataEnd = false;

                while (!cdataEnd)
                {
                    c = xmlFile[xmlPosition++];
                    if (c == 0 || xmlPosition > xmlSize) return false;

                    //                  numRead = fread(&c,1,1,f); if (numRead != 1) return false;
                    if (c == ']')
                    {
                        c = xmlFile[xmlPosition++];
                        if (c == 0 || xmlPosition > xmlSize) return false;

                        //                      numRead = fread(&c,1,1,f); if (numRead != 1) return false;
                        if (c == ']')
                        {
                            c = xmlFile[xmlPosition++];
                            if (c == 0 || xmlPosition > xmlSize) return false;

                            //          numRead = fread(&c,1,1,f); if (numRead != 1) return false;
                            if (c == '>')
                            {
                                cdataEnd = true;
                            }
                            else
                            {
                                theString[pos++] = ']';
                                theString[pos++] = ']';
                                theString[pos++] = c;
                            }
                        }
                        else
                        {
                            theString[pos++] = ']';
                            theString[pos++] = c;
                        }
                    }
                    else
                    {
                        theString[pos++] = c;
                    }
                }
                nonCdata = &(theString[pos]);
            }
            else
            {
                // the '<' we encountered was the start of the next tag, backtrack and return
                xmlPosition-=8;
                theString[pos] = 0;
                return true;
            }
        }
    }

    // } __except(catchall(GetExceptionCode(),"XML_Read_String")) {}
    // can't get here
    return false;
}



time_t mktime_utc(const struct tm *tm)
{
    time_t result = 0;

    /* We do allow some ill-formed dates, but we don't do anything special
    with them and our callers really shouldn't pass them to us.  Do
    explicitly disallow the ones that would cause invalid array accesses
    or other algorithm problems. */
    if (tm->tm_mon < 0 || tm->tm_mon > 11 || tm->tm_year < 70)
    return (time_t) -1;

    /* Convert to a time_t. */
    for (int i = 1970; i < tm->tm_year + 1900; i++) result += 365 + ISLEAP(i);
    for (int i = 0; i < tm->tm_mon; i++) result += MONTHDAYS[i];
    if (tm->tm_mon > 1 && ISLEAP(tm->tm_year + 1900)) result++;
    result = 24 * (result + tm->tm_mday - 1) + tm->tm_hour;
    result = 60 * result + tm->tm_min;
    result = 60 * result + tm->tm_sec;

    // leap seconds (AMP) -- table lists every time that one was added.
    for( const time_t* ppp = LEAP_SECONDS; *ppp; ++ppp )
    {
        if ( *ppp <= result ) result += 1;
    }

    return result;
}

time_t XML_Read_Time()
{
    char buf[100];
    FILE_PEEK( buf );

    //int oldpos = xmlPosition;
    int YYYY,MM,DD,hh,mm,ss,n;
    int rc = sscanf(buf, "%d-%d-%dT%d:%d:%d%n", &YYYY,&MM,&DD,&hh,&mm,&ss,&n);
    if (rc == 7) xmlPosition+=n;
    tm t;
    t.tm_year = YYYY - 1900;
    t.tm_mon = MM - 1;
    t.tm_mday = DD;

    t.tm_hour = hh;
    t.tm_min = mm;
    t.tm_sec = ss;

    t.tm_isdst = 0; //standard

    time_t time = mktime_utc(&t);

    return time;
}



void XML_Count()
{
    bool valid = XML_GetNextTag();
    while (valid)
    {
        if ( is_tag_name( "wpt" ) ) countStruct.numWaypoints++;

        if ( is_tag_name( "trk" ) )
        { //trk
            countStruct.numPoints[countStruct.numTracks] = 0;
            countStruct.numTrksegs[countStruct.numTracks] = 0;
            while ( valid && ! is_tag_name( "/trk" ) )
            {
                if ( is_tag_name( "trkseg" ) )
                { // (real track)
                    countStruct.numTrksegs[countStruct.numTracks]++;
                    while ( valid && ! is_tag_name( "/trkseg" ) )
                    {
                        if ( is_tag_name( "trkpt" ) )
                        { //trkpoint
                            countStruct.numPoints[countStruct.numTracks]++;
                        } //trkpt
                        valid = XML_GetNextTag();
                    } //trkseg
                }
                valid = XML_GetNextTag();
            }
            if (countStruct.numPoints[countStruct.numTracks] > 0)
            {
                countStruct.numTracks++;
            }
        } // trk

        if ( is_tag_name( "rte" ) )
        { //rte
            countStruct.numPoints[countStruct.numTracks] = 0;
            while (valid && ! is_tag_name( "/rte" ) )
            {
                if ( is_tag_name( "rtept" ) )
                { //rtept
                    countStruct.numPoints[countStruct.numTracks]++;
                } //rtept
                valid = XML_GetNextTag();
            }
            if (countStruct.numPoints[countStruct.numTracks] > 0)
            {
                countStruct.numTracks++;
            }
        } // rte

        valid = XML_GetNextTag();
    }
    xmlPosition = 0;
}


int XML_Write_Header( FILE *f )
{
    fputs(  "<?xml version=\"1.0\" standalone=\"yes\"?>\n"
            "<gpx\n"
            "  xmlns=\"http://www.topografix.com/GPX/1/0\"\n"
            "  version=\"1.0\" creator=\"TopoFusion ", f );
    fputs(  GPX_OUTPUT_FORMAT_VERSION, f );
    fputs(  "\"\n"
            "  xmlns:TopoFusion=\"http://www.TopoFusion.com\"\n"
            "  xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\"\n"
            "  xsi:schemaLocation=\"http://www.topografix.com/GPX/1/0 "
            "http://www.topografix.com/GPX/1/0/gpx.xsd "
            "http://www.TopoFusion.com "
            "http://www.TopoFusion.com/topofusion.xsd\">\n", f );
    return 0;
}


/*
 * Allocate memory for the layer objects.
 * If anything goes wrong, this frees the xmlFile, plus all local allocations.
 */
int allocate( const countStruct_type& countStruct, kjb::TopoFusion::layer* l )
{
    using kjb::TopoFusion::pt;
    using kjb::TopoFusion::waypoint;
    using kjb::TopoFusion::track;

    if (countStruct.numTracks != 0)
    {
        KJB( l->tracks = N_TYPE_MALLOC( track, countStruct.numTracks ) );
        if ( 0 == l -> tracks )
        {
            kjb_c::add_error( "%s: error: tracks bad alloc", __func__ );
            kjb_c::kjb_free( xmlFile );
            return kjb_c::ERROR;
        }
    }
    l->numTracks = countStruct.numTracks;

    if (countStruct.numWaypoints != 0)
    {
        KJB(l->waypoints = N_TYPE_MALLOC(waypoint, countStruct.numWaypoints));
        if ( 0 == l -> waypoints )
        {
            kjb_c::add_error( "%s: error: waypoints bad alloc", __func__ );
            kjb_c::kjb_free( l -> tracks );
            kjb_c::kjb_free( xmlFile );
            return kjb_c::ERROR;
        }
    }
    l->numWaypoints = countStruct.numWaypoints;

    for (int i=0;i<countStruct.numTracks;i++)
    {
        using namespace kjb_c;
        initTrack(&(l->tracks[i]));
        l->tracks[i].s.numPoints = countStruct.numPoints[i];
        l->tracks[i].s.points = N_TYPE_MALLOC(pt, countStruct.numPoints[i]);
        if ( 0 == l->tracks[i].s.points )
        {
            add_error( "%s: error: tracks-%d bad alloc", __func__, i );
            for(int j=i-1; 0 <= j; --j) kjb_free( l->tracks[j].s.points );
            kjb_free( l -> tracks );
            kjb_free( xmlFile );
            return ERROR;
        }
        const pt ZP(kjb::TopoFusion::make_pt(0, 0, 0, 0));
        std::fill_n( l->tracks[i].s.points, countStruct.numPoints[i], ZP );
    }
    return kjb_c::NO_ERROR;
}


// read entire file.  postcondition:  if successful, xmlFile owns heap memory.
int read_gpx_payload( const std::string& filename )
{
    if ( ! kjb_c::is_file( filename.c_str() ) )
    {
        DbgLog( "%s: Unable to open '%s'\n", __func__, filename.c_str() );
        return kjb_c::ERROR;
    }

    // get the size of the file, allocate a buffer for it
    kjb::File_Ptr_Read f( filename );
    kjb_c::kjb_fseek(f,0,SEEK_END);
    xmlSize = kjb_c::kjb_ftell(f);
    kjb_c::kjb_fseek(f,0,SEEK_SET);

    kjb_c::kjb_free( xmlFile );
    KJB( xmlFile = STR_MALLOC( xmlSize + 100 ) );
    KJB( NRE( xmlFile ) );

    // read the whole file
    long numRead = kjb_c::kjb_fread(f,xmlFile,xmlSize);
    if (numRead != xmlSize)
    {
        kjb_c::kjb_free(xmlFile);
        kjb_c::set_error( "%s: Unable to read '%s'\n", __func__,
                                                    filename.c_str() );
        return kjb_c::ERROR;
    }
    std::fill_n( xmlFile+xmlSize, 100, '\0' );

    return kjb_c::NO_ERROR;
}


bool XML_Read_Waypoint( kjb::TopoFusion::layer* l, int* wayptIdx )
{ //wpt
    ASSERT( wayptIdx );

    bool valid;
    XML_ParseTag();
    double  lat = XML_GetAttrib_Double("lat"),
            lon = XML_GetAttrib_Double("lon");
    LLtoUTM(23, lat, lon, l -> waypoints[ *wayptIdx ].point );
    initWaypoint( & l -> waypoints[ *wayptIdx ] );

    // default name is its index -- later we override if it has a name tag.
    kjb_c::kjb_sprintf( l -> waypoints[ *wayptIdx ].name,
                        kjb::TopoFusion::MAX_TRACKSTRING, "%d", *wayptIdx );

    for( valid = XML_GetNextTag();
                    valid && ! is_tag_name( "/wpt" );
                                valid = XML_GetNextTag() )
    {
        if ( is_tag_name( "name" ) )
        {
            XML_Read_String();
            kjb_c::kjb_strncpy( l -> waypoints[ *wayptIdx ].name,
                                theString, kjb::TopoFusion::MAX_TRACKSTRING );
        }

        if ( is_tag_name( "ele" ) )
        {
            l -> waypoints[ *wayptIdx ].point.ele = float( XML_Read_Double() );
        }

        // attributes we throw away
        if ( is_tag_name( "desc"    ) ) XML_Read_String();
        if ( is_tag_name( "cmt"     ) ) XML_Read_String();
        if ( is_tag_name( "url"     ) ) XML_Read_String();
        if ( is_tag_name( "urlname" ) ) XML_Read_String();
        if ( is_tag_name( "type"    ) ) XML_Read_String();
        if ( is_tag_name( "sym"     ) ) XML_Read_String();
        if ( is_tag_name( "time"    ) ) XML_Read_Time();
    }

    ++*wayptIdx;

    return valid;
}


bool XML_Read_Track( kjb::TopoFusion::layer* l, int* trkIdx )
{
    ASSERT( l );
    ASSERT( trkIdx );

    bool valid = true;
    int ptIdx = 0;
    kjb::TopoFusion::track *trak = & l -> tracks[ *trkIdx ];

    for ( ; valid && ! is_tag_name( "/trk" ) ; valid = XML_GetNextTag() )
    {
        if ( is_tag_name( "name" ) )
        {
            XML_Read_String();
            kjb_c::kjb_strncpy( trak -> name, theString,
                                            kjb::TopoFusion::MAX_TRACKSTRING );
        }

        if ( is_tag_name( "cmt"     ) ) XML_Read_String();
        if ( is_tag_name( "desc"    ) ) XML_Read_String();
        if ( is_tag_name( "src"     ) ) XML_Read_String();
        if ( is_tag_name( "url"     ) ) XML_Read_String();
        if ( is_tag_name( "urlname" ) ) XML_Read_String();

        if ( is_tag_name( "TopoFusion:color" ) )
        {
            char buf[100];
            FILE_PEEK( buf );

            int n;
            unsigned int ui; // could have replaced %x with %*x
            int rc = sscanf(buf, "%x</TopoFusion:color>%n", &ui, &n);
            if (rc /* == 2 */ >= 1 ) xmlPosition+=n;
        }

        if ( is_tag_name( "trkseg" ) )
        {
            /* SIMPLIFY PT 2012 JAN 21
             * I do not understand what is going on here. AMP.
            if (ptIdx > 0)
                trk->s.points[ptIdx].startnew = true;
            */

            while (valid && ! is_tag_name( "/trkseg" ))
            {
                if ( is_tag_name( "trkpt" ) )
                { //trkpoint
                    kjb::TopoFusion::pt *point = & trak->s.points[ptIdx] ;
                    XML_ParseTag();
                    double  lon = XML_GetAttrib_Double("lon"),
                            lat = XML_GetAttrib_Double("lat");

                    kjb::TopoFusion::LLtoUTM(23,lat, lon, *point);   // WGS 84

                    point->ele = kjb::TopoFusion::NO_ELEV;

                    if (! theTag.selfEnding)
                    {
                        while( valid && ! is_tag_name( "/trkpt" ) )
                        {
                            XML_ParseTag();

                            if ( is_tag_name( "ele" ) )
                            {
                                point->ele = float(XML_Read_Double());
                            }

                            if ( is_tag_name( "time" ) ) XML_Read_Time();

                            valid = XML_GetNextTag();
                        }
                    }
                    ptIdx++;
                } //trkpt
                valid = XML_GetNextTag();
            } //trkseg
        }
    }
    if (ptIdx>0) ++*trkIdx; // don't add tracks lacking trackpoints

    return valid;
} // trk


bool XML_Read_Route( kjb::TopoFusion::layer* l, int* trkIdx )
{
    ASSERT( l );
    ASSERT( trkIdx );

    bool valid = true;
    int ptIdx = 0;
    kjb::TopoFusion::track *trk = &(l->tracks[ *trkIdx ]);

    for ( ; valid && ! is_tag_name( "/rte" ); valid = XML_GetNextTag() )
    {
        if ( is_tag_name( "name" ) )
        { //name
            XML_Read_String();
            kjb_c::kjb_strncpy( trk -> name, theString,
                                            kjb::TopoFusion::MAX_TRACKSTRING );
        }

        if ( is_tag_name( "cmt"     ) ) XML_Read_String();
        if ( is_tag_name( "desc"    ) ) XML_Read_String();
        if ( is_tag_name( "src"     ) ) XML_Read_String();
        if ( is_tag_name( "url"     ) ) XML_Read_String();
        if ( is_tag_name( "urlname" ) ) XML_Read_String();

        if ( is_tag_name( "rtept" ) )
        { //trkpoint
            kjb::TopoFusion::pt &ppp = trk -> s.points[ ptIdx ];
            XML_ParseTag();
            double  lon = XML_GetAttrib_Double("lon"),
                    lat = XML_GetAttrib_Double("lat");

            kjb::TopoFusion::LLtoUTM(23,lat, lon, ppp);   // WGS 84
            ppp.ele = kjb::TopoFusion::NO_ELEV;

            if ( ! theTag.selfEnding )
            {
                while ( valid && ! is_tag_name( "/rtept" ) )
                {
                    XML_ParseTag();

                    if ( is_tag_name( "ele" ) )
                    {
                        ppp.ele = float(XML_Read_Double());
                    }

                    if ( is_tag_name( "time" ) ) XML_Read_Time();

                    valid = XML_GetNextTag();
                }
            }

            ptIdx++;
        } //rtept
    }

    // don't add tracks with no trackpoints
    if (ptIdx>0) ++*trkIdx;

    return valid;
} // rte


bool detect_ignorable_tag_and_discard()
{
    if (    is_tag_name( "name"                     )
        ||  is_tag_name( "desc"                     )
        ||  is_tag_name( "author"                   )
        ||  is_tag_name( "email"                    )
        ||  is_tag_name( "url"                      )
        ||  is_tag_name( "urlname"                  )
        ||  is_tag_name( "keywords"                 )
        ||  is_tag_name( "TopoFusion:picDirectory"  ) )
    {
        XML_Read_String();
    }
    else if ( is_tag_name( "TopoFusion:picOffset" ) )
    {
        XML_Read_Long();
    }
    else if ( is_tag_name( "TopoFusion:color" ) )
    {
        char buf[100];
        FILE_PEEK( buf );
        unsigned int dummy;
        int n, rc = sscanf(buf, "%x</TopoFusion:color>%n", &dummy, &n);
        if (rc == 2)
            xmlPosition+=n;
    }
    else if ( is_tag_name( "TopoFusion:graph" ) )
    {
        char buf[100];
        FILE_PEEK( buf );
        int tmp, n, rc = sscanf(buf, "%d</TopoFusion:graph>%n", &tmp, &n);
        if (rc == 2)
            xmlPosition+=n;
    }
    else if ( is_tag_name( "time" ) )
    {
        XML_Read_Time();
    }
    else if (   is_tag_name( "?xml"     )
            ||  is_tag_name( "gpx\n"    )
            ||  is_tag_name( "/gpx"     ) )
    {
        ; // these are expected, but we neglect to validate their locations.
    }
    else
    {
        return false;
    }

    return true;
}

} // end anonymous namespace


namespace kjb
{
namespace TopoFusion
{

/**
 * @brief read a track from a named file into the pointed-to layer
 * @param[in]   filename    name of the file to read
 * @param[out]  l           pointer to layer into which to write the track
 * @return kjb_c::NO_ERROR or ERROR as appropriate
 */
int readTrack_GPX( const std::string& filename, layer *l )
{
    int trkIdx = 0, wayptIdx = 0;

    KJB( NRE( l ) );

    // read entire file contents into char buffer xmlFile (on the heap).
    KJB( ERE( read_gpx_payload( filename ) ) );

    // pointer to the front of unscanned characters in xmlFile, i.e., input
    xmlPosition = 0;

    initLayer(l);
    kjb_c::kjb_strncpy( l->filename, filename.c_str(), LAYER_FN_SIZE );
    l->filename[ LAYER_FN_SIZE-1 ] = 0;

    // count the number of waypoints and tracks; store globally.
    countStruct.numWaypoints = 0;
    countStruct.numTracks = 0;
    XML_Count();

    KJB( ERE( allocate( countStruct, l ) ) );

    while( XML_GetNextTag() )
    {
        if ( detect_ignorable_tag_and_discard() )
        {
            continue;
        }
        else if ( is_tag_name( "wpt" ) )
        {
            XML_Read_Waypoint( l, &wayptIdx );
        }
        // can have a <trk></trk>, so make sure counter found some points
        else if ( is_tag_name( "trk" ) && countStruct.numTracks>0 )
        {
            XML_Read_Track( l, &trkIdx );
        }
        // can have a <rte></rte>, so make sure counter found some points
        else if ( is_tag_name( "rte" ) && countStruct.numTracks>0 )
        {
            XML_Read_Route( l, &trkIdx );
        }
        else
        {
            DbgLog( "Warning: unexpected tag %s!\n", theTag.name );
        }
    }

    kjb_c::kjb_free(xmlFile);
    xmlFile=0;

    return kjb_c::NO_ERROR;
}



/**
 * @brief write a track (stored in the indicated layer) into a named file
 * @param filename  name of the file to write to
 * @param l         layer containing the tracks to be written
 * @return kjb_c::NO_ERROR or ERROR as appropriate
 * @throws kjb::IO_error for certain error conditions.
 */
int writeTrack_GPX (const std::string& filename, const layer& l )
{
    kjb::File_Ptr_Write f( filename );
    XML_Write_Header(f);

    for (int i=0; i<l.numWaypoints; ++i)
    {
        double lat, lon;
        UTMtoGPXLL(23, l.waypoints[i].point, lat, lon);
        kjb_c::kjb_fprintf (f,"<wpt " LATLONF ">\n",lat,lon);

        if (l.waypoints[i].point.ele != NO_ELEV)
        {
            kjb_c::kjb_fprintf (f," <ele>%f</ele>\n",l.waypoints[i].point.ele);
        }

        XML_Write_StringTag(f,l.waypoints[i].name,"name");
        kjb_c::kjb_fputs (f,"</wpt>\n");
    }


    for (int i=0; i<l.numTracks; ++i)
    {
        if (l.tracks[i].s.numPoints <=0) continue;
        kjb_c::kjb_fputs (f,"<trk>\n");

        XML_Write_StringTag(f,l.tracks[i].name,"name");

        kjb_c::kjb_fputs (f,"  <trkseg>\n");
        for (int j=0;j<l.tracks[i].s.numPoints;j++)
        {
            double lat, lon;
            UTMtoGPXLL(23,l.tracks[i].s.points[j], lat, lon);
            if (lon < -180)
            {
                kjb_c::set_error( "Invalid longitude, lon=%f", lon );
                return kjb_c::ERROR;
            }
            kjb_c::kjb_fprintf (f,"    <trkpt " LATLONF ">\n", lat, lon);
            if (l.tracks[i].s.points[j].ele != NO_ELEV)
            {
                kjb_c::kjb_fprintf (f,
                        "      <ele>%f</ele>\n",l.tracks[i].s.points[j].ele);
            }
            kjb_c::kjb_fputs (f,"    </trkpt>\n");
        }
        kjb_c::kjb_fputs (f,"  </trkseg>\n</trk>\n");
    }

    kjb_c::kjb_fputs (f,"</gpx>\n");
    return kjb_c::NO_ERROR;
}


} // end namespace TopoFusion
} // end namespace kjb
