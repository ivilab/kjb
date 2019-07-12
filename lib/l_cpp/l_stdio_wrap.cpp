/**
 * @file
 * @brief implementation of the unzipping trickery used by File_Ptr_Smart_Read
 * @author Andrew Predoehl
 *
 * @todo fix system call, which would be better if it were kjb_system
 */
/*
 * $Id: l_stdio_wrap.cpp 15831 2013-10-22 02:07:55Z predoehl $
 */

#include <l/l_sys_lib.h>
#include <l/l_error.h>
#include <l/l_global.h>
#include <l_cpp/l_util.h>
#include <l_cpp/l_stdio_wrap.h>

#include <vector>

/*
 * The macro TEMP_FN_TEMPLATE defines a temporary filename template as
 * required by the system, e.g., mkstemp(3).  The messy conditional compilation
 * handles the ugly fact that SOME machines (namely v11) have a puny, feeble
 * /tmp dir, i.e., one that does not have enough space to be useful.
 *
 * As a workaround, on v11 you can use the /scratch directory for temporary
 * files.  If you are going to run your code on v11, you should set the
 * #if flag below to zero.  Please do not commit this change!  Default behavior
 * should be to use TEMP_DIR.
 *
 * Note that there is no #else clause, by design.
 */
#if 1 /* <=== Set this to 0 if your machine has a puny /tmp dir (like V11),
              but do not commit this change! */
#define TEMP_FN_TEMPLATE TEMP_DIR DIR_STR "libkjb-XXXXXX"
#elif defined(UNIX_SYSTEM)
#define TEMP_FN_TEMPLATE "/net/v07/scratch/libkjb-XXXXXX"
#endif


namespace {

struct Zip_Kind {
    const std::string suffix, catcmd;
};

const Zip_Kind zips[] = {
    { ".gz",    "zcat"  },
    { ".bz2",   "bzcat" },
    { "",       ""      } // SENTINEL END OF LIST
};

bool is_nonchar( int ccc )
{
    return      EOF == ccc
            ||  kjb_c::INTERRUPTED == ccc
            ||  kjb_c::ERROR == ccc;
}


std::vector<char> default_template()
{
    const std::string sfn = TEMP_FN_TEMPLATE;
    std::vector<char> vfn(sfn.begin(), sfn.end()); // copy all the characters
    vfn.push_back('\0'); // append a C-style zero character to mark the end
    return vfn;
}

}


#ifdef UNIX_SYSTEM
namespace kjb {

Temporary_File::Temporary_File()
{
    std::vector<char> filename = default_template();
    char* p_fname = & filename.front();

    int fd = mkstemp( p_fname );
    ETX_2( -1 == fd, "Call to mkstemp failed in class Temporary_File" );
    File_Ptr_Read_Plus fplus( m_filename = p_fname );

    int rc = ::close( fd );
    ETX_2( -1 == rc, "Unable to close file descriptor in Temporary_File" );
    File_Ptr::swap( fplus );
}



void File_Ptr_Smart_Read::attempt_unzip( const std::string& filename )
{
    for( const Zip_Kind* zzz = zips; zzz -> suffix.size(); ++zzz )
    {
        const std::string cmprsd = filename + zzz -> suffix;
        if ( kjb_c::is_file( cmprsd.c_str() ) )
        {
            temp.reset( new Temporary_File() );
            const std::string unzip_cmd = zzz -> catcmd + " " + cmprsd
                                            + " >" + temp -> get_filename();

            // we ignore return value because bzcat doesn't play by the rules.
            // kjb_system() is failing -- not sure why.  Std. system() is ok.
            //kjb_c::kjb_system( unzip_cmd.c_str() );
            system( unzip_cmd.c_str() );

            File_Ptr_Read file_pointer_to_expanded_file(temp->get_filename());
            File_Ptr::swap( file_pointer_to_expanded_file );
            return;
        }
    }
    throw_io_error( filename, "r" );
}


int getline( FILE* fp, std::string* line, char EOL )
{
    KJB( NRE( line ) );
    KJB( NRE( fp ) );

    // Return error if file is already messed up or we cannot write into line
    if ( ferror( fp ) )
    {
        return kjb_c::ERROR;
    }

    // Try to read the first character (special case)
    int ccc = kjb_c::kjb_fgetc( fp );
    if ( is_nonchar( ccc ) || ferror( fp ) )
    {
        return kjb_c::ERROR;
    }
    line -> push_back( static_cast< char >( ccc ) );

    // Read more characters, if any.  Normal exit conditions are ccc==EOF, EOL.
    while ( ! ferror( fp ) && ! is_nonchar( ccc = kjb_c::kjb_fgetc( fp ) ) )
    {
        line -> push_back( static_cast< char >( ccc ) );
        if ( EOL == ccc ) break;
    }

    if ( ferror( fp ) )
    {
        return kjb_c::ERROR;
    }
    return EOL == ccc || EOF == ccc ? kjb_c::NO_ERROR : kjb_c::ERROR;
}


Temporary_Directory::Temporary_Directory()
:   m_silence(false)
{
    std::vector<char> filename = default_template();
    char* p_fname = & filename.front();

    char *ppp = mkdtemp( p_fname );
    if ( 0 == ppp )
    {
        int errsave = errno; // paranoia
        kjb_c::set_error( "Temporary_Directory failure calling mkdtemp: "
                            "error %d, %s\n", errsave, strerror( errsave ) );
        KJB_THROW_2(kjb::IO_error, "Temporary_Directory: mkdtemp() failed");
    }
    else if ( ppp != p_fname )
    {
        KJB_THROW_2(kjb::Cant_happen, "Temporary_Directory: mkdtemp bad RV!");
    }
    m_pathname = p_fname;
}

}
#endif

