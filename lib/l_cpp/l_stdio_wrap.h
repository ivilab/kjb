/**
 * @file
 * @brief RAII wrappers on FILE* objects from stdio.h and other stdio helpers.
 * @author Andrew Predoehl
 */
/*
 * $Id: l_stdio_wrap.h 21596 2017-07-30 23:33:36Z kobus $
 */

/* =========================================================================== *
|
|  Copyright (c) 2012 by Kobus Barnard and Andrew Predoehl.
|
|  Personal and educational use of this code is granted, provided that this
|  header is kept intact, and that the authorship is not misrepresented, that
|  its use is acknowledged in publications, and relevant papers are cited.
|
|  For other use contact the author (kobus AT sista DOT arizona DOT edu).
|
|  Please note that the code in this file has not necessarily been adequately
|  tested. Naturally, there is no guarantee of performance, support, or fitness
|  for any particular task. Nonetheless, I am interested in hearing about
|  problems that you encounter.
|
* =========================================================================== */


#ifndef L_STDIO_WRAP_H_UOFARIZONA_VISION
#define L_STDIO_WRAP_H_UOFARIZONA_VISION

#include "l/l_sys_debug.h"  /* For ASSERT */
#include "l/l_sys_lib.h"
#include "l/l_sys_io.h"
#include "l/l_error.h"
#include "l/l_global.h"
#include "l_cpp/l_exception.h"

#include <string>
#include <boost/scoped_ptr.hpp>

namespace kjb
{


/**
 * @brief RAII wrapper on stdio.h FILE pointers (use a derived class though).
 *
 * If you have the misfortune of needing to use stdio.h routines for file IO,
 * consider using one of the derived classes of this class as an RAII wrapper
 * on the pointer object.  When done correctly, it means never having to call
 * the close function yourself, because the dtor will do it for you.
 *
 * This is a base class not meant to be instantiated directly.  Instead, use
 * one of the following derived classes (list not necessarily exhaustive):
 * - File_Ptr_Read
 * - File_Ptr_Write
 * - File_Ptr_Append
 * - Temporary_File
 * - File_Ptr_Smart_Read
 * - File_Ptr_Read_Plus
 */
class File_Ptr
{
    FILE* m_fp; ///< pointer to FILE object we are protecting

    File_Ptr( const File_Ptr& ); // teaser -- this is a non-copyable class
    File_Ptr& operator=( const File_Ptr& ); // teaser -- non-assignable too

protected:

    /**
     * @brief this is what we do when things go terribly wrong
     * @throws kjb::IO_error in all cases, echoing the parameters.
     */
    void throw_io_error( const std::string& pathname, const char* mode )
    {
        KJB_THROW_2( kjb::IO_error, "Unable to open " + pathname
                                                    + " with mode " + mode );
    }

    /**
     * @brief ctor needs the pathname and a mode; mode is set by derived class
     * @throws kjb::IO_error if the underlying fopen call fails
     */
    File_Ptr( const std::string& pathname, const char* mode )
    :   m_fp( kjb_c::kjb_fopen( pathname.c_str(), mode ) )
    {
        if ( 0 == m_fp )
        {
            throw_io_error( pathname, mode );
        }
    }

    /// @brief default ctor wraps a null pointer (to set up a later swap maybe)
    File_Ptr()
    :   m_fp( 0 )
    {}

    /// @brief swap the contents of two wrappers
    void swap( File_Ptr& that )
    {
        std::swap( m_fp, that.m_fp );
    }

public:

    /// @brief close file before destruction; rarely needed; safe to do twice.
    virtual int close()
    {
        int rc = kjb_c::kjb_fclose( m_fp );
        m_fp = 0;
        return rc;
    }

    /// @brief dtor can simply call close() because closing a null ptr is OK.
    virtual ~File_Ptr()
    {
        close();
    }

    /// @brief transparently use this object wherever a FILE* is used!
    operator FILE*() const
    {
        return m_fp;
    }
};

/// @brief RAII wrapper on FILE* used to read a file.
struct File_Ptr_Read : public File_Ptr
{
    File_Ptr_Read( const std::string& pathname )
    :   File_Ptr( pathname, "r" )
    {}
};

/// @brief RAII wrapper on a FILE* used to write to a file.
struct File_Ptr_Write : public File_Ptr
{
    File_Ptr_Write( const std::string& pathname )
    :   File_Ptr( pathname, "w" )
    {}
};

/// @brief RAII wrapper on FILE* used to append a file (write only at the end).
struct File_Ptr_Append : public File_Ptr
{
    File_Ptr_Append( const std::string& pathname )
    :   File_Ptr( pathname, "a" )
    {}
};

/// @brief RAII wrapper on FILE* opened with mode "r+" to read and write.
struct File_Ptr_Read_Plus : public File_Ptr
{
    File_Ptr_Read_Plus( const std::string& pathname )
    :   File_Ptr( pathname, "r+" )
    {}
};

// If you need a mode like w+ or a+ then please add a new derived class




/**
 * @brief This class safely opens a temporary file in TEMP_DIR (usually /tmp).
 *
 * This class is a wrapper around Unix system call mkstemp which securely
 * generates a unique temporary filename and opens the file too, so that there
 * is no race condition.
 *
 * This object deletes its underlying file only when the object is destroyed.
 * If the program aborts, the temporary file might not be deleted.  In that
 * case, you might want to clean up files called /tmp/libkjb-XXXXXX manually,
 * where XXXXXX denotes any arbitrary combination of six printable characters.
 *
 * Example usage:
 * @code
 * char buf[ 100 ];
 * Temporary_File tfo;
 * fputs( "A is for Absinthe", tfo );
 * rewind( tfo );
 * fgets( buf, sizeof(buf), tfo );
 * puts( buf );
 * @endcode
 */
class Temporary_File : public File_Ptr
{
#ifdef UNIX_SYSTEM

    std::string m_filename; ///< name of the temporary file we generate

public:

    /// @brief ctor creates a temporary file, open for reading and writing
    Temporary_File();

    /// @brief eliminate the temp file; safe to call twice
    int close()
    {
        int rc1 = 0, rc2 = 0;
        if ( m_filename.size() )
        {
            rc1 = File_Ptr::close();
            rc2 = kjb_c::kjb_unlink( m_filename.c_str() );
            m_filename.clear();
        }
        return rc1 | rc2;
    }

    /// @brief dtor does the same thing as close(), eliminating the file
    ~Temporary_File()
    {
        close();
    }

    /**
     * @brief get the filename of this object (if still valid)
     *
     * Iff the close() method has been previously called, then this method
     * returns a reference to
     * an empty string, indicating that the underlying file is gone.
     */
    const std::string& get_filename() const
    {
        return m_filename;
    }

#else /* UNIX_SYSTEM above, non-unix below. */
#warning "On non-Unix style systems, class Temporary_File does not work."
public:
    Temporary_File() {}
    int close() { return 0; }
    const std::string& get_filename() const { return ""; }
#endif /* UNIX_SYSTEM (or not) */
};


/**
 * @brief this class creates a temporary directory under TEMP_DIR (usu. /tmp)
 *
 * If the directory is empty at destruction time, this removes the directory.
 * If the directory is not empty, this sets errno and maybe prints a message,
 * but does not throw.  If you need to clear out the directory prior to
 * destruction
 * then consider calling the recursively_remove() method, or use
 * class Temporary_Recursively_Removing_Directory.
 *
 * The default behavior of the class is for the destructor to print if it
 * is unable to succeed.
 * If you do not want the destructor to print, regardless of error status, use
 * the method set_silence_flag().  This same method returns
 * the prior state of the flag.
 */
class Temporary_Directory
{

    std::string m_pathname;

    bool m_silence;

    // this item is non-copyable and non-assignable
    Temporary_Directory(const Temporary_Directory&); // copy ctor teaser
    Temporary_Directory& operator=(const Temporary_Directory&); // teaser

#ifdef UNIX_SYSTEM

    int do_rmdir() const
    {
        return kjb_c::kjb_rmdir( m_pathname.c_str() );
    }

    int do_rmrf() const
    {
        return kjb_c::kjb_system( ("rm -rf " + m_pathname).c_str() );
    }

    int do_the_cleaning(
        int( Temporary_Directory::* pf_cleaning )() const,
        const std::string& fail_func
    )
    {
        using namespace kjb_c;
        ASSERT( pf_cleaning );
        if ( 0 == m_pathname.size() ) return NO_ERROR;

        int rc = (this ->* pf_cleaning)();
        if ( ! is_directory( m_pathname.c_str() ) ) m_pathname.clear();

        if ( ERROR == rc )
        {
            add_error(("Temporary_Directory::"+ fail_func +" failed").c_str());
        }
        return rc;
    }

public:

    Temporary_Directory();

    /**
     * @brief delete the temporary directory (if empty) otherwise return ERROR
     *
     * Iff successful this returns kjb_c::NO_ERROR.  Safe to call twice, i.e.,
     * if this successfully removes the directory, later calls have no effect
     * but they still return NO_ERROR.  A call can fail for many reasons (see
     * rmdir(2)) but the most common is when the directory is not empty.  If
     * an initial attempt fails for this reason, and if you empty the directory
     * (or call recursively_remove()), then a later call ordinarily will
     * succeed.
     */
    int remove()
    {
        return do_the_cleaning( & Temporary_Directory::do_rmdir, __func__ );
    }

    /// @brief delete the directory and all its contents, like "rm -rf" does.
    int recursively_remove()
    {
        return do_the_cleaning( & Temporary_Directory::do_rmrf, __func__ );
    }

    /// @brief dtor executes remove(), NOT recursively_remove(), maybe prints.
    virtual ~Temporary_Directory()
    {
        int status = remove();
        if (!m_silence)
        {
            using namespace kjb_c;
            EPE(status);
        }
    }

#else /* UNIX_SYSTEM above, non-unix below. */
#warning "On non-Unix style systems, class Temporary_Directory does not work."
public:
    Temporary_Directory() {}
    int remove()                { return kjb_c::NO_ERROR; }
    int recursively_remove()    { return kjb_c::NO_ERROR; }
#endif /* UNIX_SYSTEM (or not) */

    /**
     * @brief get the filename of this object (if still valid)
     *
     * Iff either cleanup method has succeeded previously, then this method
     * returns a reference to
     * an empty string, indicating that the underlying file is gone.
     */
    const std::string& get_pathname() const
    {
        return m_pathname;
    }

    /**
     * @brief By default the dtor prints errors (if any) -- you can suppress it
     * @param new_silence_setting if false, and if the (future) destruction
     *                            step fails, the destructor will print the
     *                            relevant error message.  If true, nothing
     *                            will be printed.
     * @return previous value of the flag
     */
    bool set_silence_flag(bool new_silence_setting = true)
    {
        std::swap(m_silence, new_silence_setting);
        return new_silence_setting;
    }
};


/// @brief create a temp directory that destroys itself with "rm -rf" command.
struct Temporary_Recursively_Removing_Directory : public Temporary_Directory
{
    Temporary_Recursively_Removing_Directory()
    {
    }

    ~Temporary_Recursively_Removing_Directory()
    {
        using namespace kjb_c;
        EPE( recursively_remove() );
    }

#ifndef UNIX_SYSTEM
#warning "class Temporary_Recursively_Removing_Directory unavailable w/o Unix."
#endif
};



/**
 * @brief This class transparently opens gzipped or bzip2-ed files.
 *
 * This is like a fancier version of File_Ptr_Read, handling for you the chance
 * that you've zipped up the target file, even if you don't remember doing so.
 * I.e., this will automatically try your filename with zip-like suffixes added
 * on if the ordinary filename is not found (or otherwise not readable).
 *
 * You, the user, are not expected to append ".gz" or ".bz2" to the filename.
 * Rather, the ctor of this class takes your filename (say "apple.txt") and
 * tries to open it.  If it fails, it automatically looks for the presence of
 * zipped versions (e.g., "apple.txt.gz" and "apple.txt.bz2"), and if they are
 * found, this unzips them in a temporary file and gives you a pointer to the
 * temporary file.  This temporary file has a randomly generated filename; use
 * method get_temp_filename() to access it.  When this object is destroyed the
 * temporary file is deleted.
 *
 * In other words, if the filename given to the ctor is present, then this
 * object acts just like a File_Ptr_Read.  If not, but if filename plus a
 * common zip suffix exists, then this object decompresses to a temporary file
 * and opens that as a File_Ptr_Read.
 *
 * We do not specify the order that the compression suffixes are tested; if
 * more than one are present, any one of them might be opened.
 *
 * If the input filename is, for example, "apple.txt.bz2" and if that file is
 * present, this file will simply open the binary compressed file for you,
 * which is likely to be non-text.  That maybe wasn't what you meant.
 *
 * @todo attempt_unzip() uses system(); should be kjb_system(), but that fails.
 */
class File_Ptr_Smart_Read : public File_Ptr
{
#ifdef UNIX_SYSTEM

    boost::scoped_ptr< Temporary_File > temp;

    void attempt_unzip( const std::string& );

public:

    /// @brief build object, like File_Ptr_Read, but with optional auto unzip
    File_Ptr_Smart_Read( const std::string& filename )
    {
        try
        {
            File_Ptr_Read fp( filename );
            File_Ptr::swap( fp );
        }
        catch( kjb::IO_error& )
        {
            attempt_unzip( filename );
        }
    }

    /**
     * @brief get name of temporary file of unzipped contents (if any)
     * @return null string, if ctor found given filename, or temp file name.
     *
     * Iff the input file was found in compressed form, this returns the name
     * of the file.  You could link to it if you want to inhibit its later
     * deletion.  Whereas if the input file was found with an unmodified name,
     * this returns an empty string.
     */
    std::string get_temp_filename() const
    {
        if ( temp )
        {
            return temp -> get_filename();
        }
        return "";
    }

#else /* UNIX_SYSTEM above, non-unix below. */
#warning "On non-Unix style systems, class File_Ptr_Smart_Read does not work."
public:
    File_Ptr_Smart_Read( const std::string& fn ) : File_Ptr_Read( fn ) {}
    std::string get_temp_filename() const { return ""; }
#endif /* UNIX_SYSTEM (or not) */
};


/**
 * @brief Like C's fgets but with std::string, or C++'s getline but with FILE*
 *
 * C has the fgets(3) function, but requires that you know the maximum length
 * of an input line.  C++ has the getline function for a string, but requires
 * that you use streams.  This function lets you read a line from a FILE* into
 * a string, giving the best of both libraries.
 *
 * A "line" is defined as a string of characters read from the FILE* until we
 * hit EOF or we read the EOL character you specified (unless EOL was allowed
 * to default to newline).  Just as fgets does, if an EOL character is read, we
 * include it in the string as the last character.
 *
 * @param [in]  fp      File from which to read
 *
 * @param [out] line    Input is appended onto the back of this string.
 *                      If this pointer equals NULL, that is considered an
 *                      error and the return value will be EOF.
 *
 * @param [in] EOL      Character that delimits the line of input (we presume
 *                      such a value must exist).  As is conventional, the
 *                      default value is '\n'
 *
 * @return kjb_c::ERROR if there is an error or if we reach EOF before any
 *          characters are read; but if successful, return kjb_c::NO_ERROR.
 */
int getline( FILE* fp, std::string* line, char EOL = '\n' );


} //namespace kjb

#endif /* L_STDIO_WRAP_H_UOFARIZONA_VISION */
