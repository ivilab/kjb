/**
 * @file
 * @author Andrew Predoehl
 * @brief convert a NED elevation datafile into an image
 */
/*
 * $Id: float2img.cpp 17425 2014-08-30 00:34:38Z predoehl $
 *
 * Tab size: 4
 */

// the following macro def is indicated for getopt_long
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <l/l_def.h>
#include <l_cpp/l_exception.h>
#include <i_cpp/i_image.h>
#include <i_cpp/i_hsv.h>
#include <topo_cpp/nedget.h>

extern "C" {
#include <getopt.h> /* getopt_long */
}

#include <vector>
#include <string>
#include <map>
#include <iostream>
#include <algorithm>
#include <iterator>


extern int optind;      // declared by <unistd.h>, used for option parsing
extern char* optarg;    // declared by <unistd.h>, used for option parsing

namespace {




const std::string format =
"\nFormat: float2img [OPTIONS] file ...\n"
"\n"
"OPTIONS\n"
"\t" "--msbfirst\t" "Flags to indicate byte order of floating point\n"
"\t" "--lsbfirst\t" "numbers in all the input file(s).  If omitted, the\n"
"\t" "-m\t" "\t"    "program tries to autodetect the byteorder.  If more\n"
"\t" "-l\t" "\t"    "than one of these options are specified, the rightmost\n"
"\t" "\t"   "\t"    "overrides the others.  Options -m and -l are synonyms\n"
"\t" "\t"   "\t"    "for --msbfirst and --lsbfirst, respectively.\n"
"\n"
"\t" "--absolute\t" "Flags to indicate the kind of transformation.\n"
"\t" "--relative\t" "Default is --absolute, which transforms elevations into\n"
"\t" "-a\t" "\t"    "a hue corresponding to its absolute elevation, red=low\n"
"\t" "-r\t" "\t"    "and blue=high.  Option --relative transforms\n"
"\t" "\t"   "\t"    "elevations into a normalized grayscale range, with\n"
"\t" "\t"   "\t"    "black at the lowest point and white at the highest.\n"
"\t" "\t"   "\t"    "If these two options are both specified, the rightmost\n"
"\t" "\t"   "\t"    "overrides any others.  Options -a and -r are synonyms\n"
"\t" "\t"   "\t"    "for --absolute and --relative, respectively.\n"
"\n"
"\t" "--output fn\tSpecifies one output filename 'fn' instead of default.\n"
"\t" "-o fn\t\t"    "If used, there must be one such option for each input\n"
"\t" "\t"   "\t"    "file, in the corresponding order of the input files.\n"
"\n"
"\t" "--help\t\t"   "Print this synopsis and exit.\n"
"\t" "-h\n"
"\n"
"file ...\t" "Name of input file(s) of binary, single-precision floating\n"
"\t" "\t"   "point numbers.  File sizes must indicate a square grid.\n"
"\t" "\t"   "If a byteorder option is specified, all files must have the\n"
"\t" "\t"   "same byteorder.  (Not so, if using default autodetection.)\n"
"\n"
"Duplicated input filenames will be culled, each file processed just once.\n"
"\n"
"Missing data is shown by black pixels in --absolute style output, and by a\n"
"checkerboard pattern of black and white pixels in --relative style output.\n"
"\n"
"Output will be written to one or more files.  The output filename(s), if\n"
"not specified with option --output, will be built from the input\n"
"filename(s) like so:  if an input filename is something like 'x.flt' then\n"
"the corresponding default output filename is 'x.jpeg'; if the '.flt'\n"
"exension is absent then '.jpeg' is simply appended.\n"
"\n"
"BUGS\t"    "The scheme for generating default output filenames can produce\n"
"\t"        "duplicates, or clobber other files, but this program does\n"
"\t"        "nothing about that.\n";




// command line options -- keep this synched with shortops.
const struct option opz[] =
{
    { "lsbfirst",   0, 0, 'l' },
    { "msbfirst",   0, 0, 'm' },
    { "relative",   0, 0, 'r' },
    { "absolute",   0, 0, 'a' },
    { "output",     1, 0, 'o' },
    { "help",       0, 0, 'h' },
    { 0, 0, 0, 0 }
};

const char* shortops = "lmrao:h";   // keep this synched with opz[]


const kjb_c::Pixel BLACK = kjb::PixelRGBA(0,0,0);


// global structure storing program options and input, output filenames
struct Params
{
    typedef std::vector< std::string > VS;
    typedef std::pair< VS::const_iterator, VS::const_iterator > Ref;

    enum kjb::NED13_FLOAT_AUTODETECT byteorder; // input byteorder
    bool rel_ele;   // is the output to be shown in relative-elevation form?
    VS ifilenames, ofilenames;  // input filenames, output filenames

    int validate_filenames();

    static std::string output_fn( const std::string& const_input_fn )
    {
        std::string input_fn( const_input_fn );
        int MIN4 = input_fn.size() - 4;
        if( 0 < MIN4 && ".flt" == input_fn.substr( MIN4 ) )
        {
            input_fn.resize( MIN4 );
        }
        return input_fn += ".jpeg";
    }

    // You can iterate through the input and output filename pairs
    struct const_IOiter
    :   public std::iterator< std::input_iterator_tag, void, void, void, Ref >
    {
        VS::const_iterator m_in_it, m_out_it;
        const_IOiter( Ref x ) : m_in_it( x.first ), m_out_it( x.second ) {}
        const_IOiter operator++() { ++m_in_it; ++m_out_it; return *this; }
        Ref operator*() const { return std::make_pair( m_in_it, m_out_it ); }
        bool operator!=( const const_IOiter& jjj ) const
        {
            return m_in_it != jjj.m_in_it || m_out_it != jjj.m_out_it;
        }
    };

    const_IOiter fn_begin() const
    {
        return std::make_pair( ifilenames.begin(), ofilenames.begin() );
    }

    const_IOiter fn_end() const
    {
        return std::make_pair( ifilenames.end(), ofilenames.end() );
    }

    void db_print() const
    {
        std::cout << "#input fns: " << ifilenames.size()
                << " #output fns: " << ofilenames.size() << '\n';
        for( size_t iii = 0; iii < ifilenames.size(); ++iii )
        {
            std::cout << ' ' << ifilenames[ iii ];
        }
        std::cout << '\n';
        for( size_t iii = 0; iii < ofilenames.size(); ++iii )
        {
            std::cout << ' ' << ofilenames[ iii ];
        }
        std::cout << '\n';
    }
};

const Params* g_params; // global pointer to the invocation set of Params


// are the input filenames ok?  return kjb_c::ERROR if not.
int Params::validate_filenames()
{
    if ( ifilenames.empty() )
    {
        kjb_c::set_error( "Error:  no input filenames were provided.\n" );
        return kjb_c::ERROR;
    }

    if ( ! ofilenames.empty() && ofilenames.size() != ifilenames.size() )
    {
        kjb_c::set_error( "Error:  unable to pair up %u input filename(s) "
                                "with %u output filename(s).\n",
                                ifilenames.size(), ofilenames.size() );
        return kjb_c::ERROR;
    }

    // look for duplicate input filenames
    std::map< std::string, int > infn;
    std::vector< size_t > kill_list;
    size_t jjj = 1;
    for( VS::const_iterator iii = ifilenames.begin(); iii != ifilenames.end();
                                                                ++iii, ++jjj )
    {
        if( infn[ *iii ] ) kill_list.push_back( jjj ); else infn[ *iii ] = jjj;
    }

    // remove duplicated input, output filenames and print a warning
    for( ; ! kill_list.empty(); kill_list.pop_back() )
    {
        std::cerr << "Warning:  duplicated filename "
                                    << ifilenames[ kill_list.back() - 1 ]
                                    << " detected -- ignoring repetitions\n";

        if ( ! ofilenames.empty() )
        {
            std::cerr << "Warning:  also ignoring corresponding output "
                        "filename " << ofilenames[ kill_list.back() - 1 ]
                        << ".\n";
            ofilenames.erase( ofilenames.begin() + kill_list.back() - 1 );
        }

        ifilenames.erase( ifilenames.begin() + kill_list.back() - 1 );
    }

    // generate default filenames if none have been given in the options.
    // they might not be unique, e.g., input abc.flt abc; not my problem.
    if ( 0 == ofilenames.size() )
    {
        std::transform( ifilenames.begin(), ifilenames.end(),
                        std::back_inserter( ofilenames ),
                        std::ptr_fun( Params::output_fn ) );
    }

    return kjb_c::NO_ERROR;
}


// process command line args and options, using getopt_long.  Output in params.
int read_opts_impl( int argc, char* const argv[], Params* params )
{
    KJB( NRE( argv ) );
    KJB( NRE( params ) );

    params -> byteorder = kjb::NED_AD_UNCERTAIN;

    for( int ccc = 0; ccc != -1; )
    {
        int index;
        ccc = getopt_long( argc, argv, shortops, opz, &index );
        if ( 'l' == ccc )
        {
            params -> byteorder = kjb::NED_AD_LSBFIRST;
        }
        else if ( 'm' == ccc )
        {
            params -> byteorder = kjb::NED_AD_MSBFIRST;
        }
        else if ( 'r' == ccc )
        {
            params -> rel_ele = true;
        }
        else if ( 'a' == ccc )
        {
            params -> rel_ele = false;
        }
        else if ( 'o' == ccc )
        {
            params -> ofilenames.push_back( optarg );
        }
        else if ( 'h' == ccc )
        {
            std::cout << format;
            kjb_c::kjb_exit( EXIT_SUCCESS );
        }
        else if ( '?' == ccc ) // unrecognized option
        {
            kjb_c::set_error( "Unrecognized option!\n\n" );
            return kjb_c::ERROR;
        }
        else
        {
            assert( -1 == ccc );
        }
    }

    // read input filenames
    while( optind < argc )
    {
        params -> ifilenames.push_back( argv[ optind++ ] );
    }

    KJB( ERE( params -> validate_filenames() ) );

    return kjb_c::NO_ERROR;
}


int read_options( int argc, char* const argv[], Params* params )
{
    int rc = read_opts_impl( argc, argv, params );
    if ( kjb_c::ERROR == rc )
    {
        std::cerr << format;
    }
    return rc;
}


void absolute_ele( const Params::Ref& iopair )
{
    // Pull input file points into a matrix
    kjb::Matrix elevations;
    KJB( ETX( kjb::get_ned_matrix(
                    * iopair.first, &elevations, g_params -> byteorder ) ) );

    const kjb::Matrix* const cele = &elevations;
    kjb::Image nim( cele -> get_num_rows(), cele -> get_num_cols() );

    const double MtMcK = 6000.0; // rough height of Mt. McKinley, in meters
    for( int row = 0; row < cele -> get_num_rows(); ++row )
    {
        for( int col = 0; col < cele -> get_num_cols(); ++col )
        {
            nim.at( row, col )
                    = kjb::PixelHSVA( cele -> at( row, col ) / MtMcK, 1, 1 );

            // show missing data as black
            if ( kjb::NED_MISSING == cele -> at( row, col ) )
            {
                nim.at( row, col ) = BLACK;
            }
        }
    }

    nim.write( * iopair.second );
}


void relative_ele( const Params::Ref& iopair )
{
    typedef std::deque< float > FD;
    FD eld;

    // Pull input file points into a deque
    KJB( ETX( kjb::get_ned_fdeq(
                            * iopair.first, &eld, g_params -> byteorder ) ) );

    /*
     * Scan for max, min elev's, but exclude loci storing the MISSING sentinel.
     * Also, store a mask (row-major order) of where the missing data are.
     */
    std::vector< bool > missing_mask( eld.size(), 0 );
    std::vector< bool >::iterator mmm = missing_mask.begin();
    float emax = -FLT_MAX, emin = FLT_MAX;
    for( FD::const_iterator iii = eld.begin(); iii != eld.end(); ++iii, ++mmm )
    {
        if ( *iii != kjb::NED_MISSING )
        {
            emax = std::max( emax, *iii );
            emin = std::min( emin, *iii );
        }
        else
        {
            *mmm = true;
        }
    }
    const double dmin( emin ), scale( 255.0 / (double( emax ) - dmin) );

    /*
     * Transfer data into a matrix form, normalizing good data and inserting a
     * checkerboard pattern, block size 2x2 pixels, where data are missing.
     * We do this in two passes, which is probably not optimal, but the code is
     * simpler this way, and I bet the cost is insignificant.
     */
    kjb::Matrix elm;
    KJB( ETX( kjb::ned_fdeq_to_matrix( eld, &elm ) ) );
    std::vector< bool >::const_iterator qqq = missing_mask.begin();

    for( int row = 0; row < elm.get_num_rows(); ++row )
    {
        for( int col = 0; col < elm.get_num_cols(); ++col, ++qqq )
        {
            double &elt = elm.at( row, col );
            if ( *qqq )
            {
                elt = (row & 02) ^ (col & 02) ? 0 : 255; // checkerboard
            }
            else
            {
                elt -= dmin;    // rescale so data uses full grayscale range
                elt *= scale;
            }
        }
    }

    // Library does lots of work here, turns matrix into grayscale image file.
    kjb::Image( elm ).write( * iopair.second );
}



}


int main( int argc, char* const argv[] ) 
{
    // Read program options and arguments; permit global read-only access.
    Params params;
    KJB( EPETE( read_options( argc, argv, &params ) ) );
    g_params = &params;

    try
    {
        std::for_each( g_params -> fn_begin(), g_params -> fn_end(),
            std::ptr_fun( g_params -> rel_ele ? relative_ele : absolute_ele )
            );
    }
    catch( kjb::Exception& eee )
    {
        eee.print_details_exit();
    }

    return EXIT_SUCCESS;
}

