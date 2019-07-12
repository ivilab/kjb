/* $Id: l_readable.cpp 17330 2014-08-19 20:49:51Z predoehl $ */

/* =========================================================================== *
|
|  Copyright (c) 1994-2008 by Kobus Barnard (author).
|
|  Personal and educational use of this code is granted, provided that this
|  header is kept intact, and that the authorship is not misrepresented, that
|  its use is acknowledged in publications, and relevant papers are cited.
|
|  For other use contact the author (kobus AT cs DOT arizona DOT edu).
|
|  Please note that the code in this file has not necessarily been adequately
|  tested. Naturally, there is no guarantee of performance, support, or fitness
|  for any particular task. Nonetheless, I am interested in hearing about
|  problems that you encounter.
|
| Authors:
|     Kyle Simek, Joseph Schlecht, Luca Del Pero
|
* =========================================================================== */

#include <l/l_sys_lib.h>
#include <l/l_string.h>
#include <l_cpp/l_util.h>
#include <l_cpp/l_exception.h>
#include <l_cpp/l_readable.h>

#include <fstream>
#include <string>


namespace kjb
{

/** 
 * @param  fname  Input file to read from.
 *
 * @throw  kjb::IO_error   Could not read from @em fname.
 * @throw  kjb::Illegal_argument  Invalid arguments in the file to read from.
 */
void Readable::read(const char* fname)
{
    KJB(NTX(fname));

    std::ifstream in(fname);
    if (in.fail())
    {
        KJB_THROW_2(IO_error, std::string(fname) + ": Could not open file");
    }

    try
    {
        read(in);
    }
    catch (const IO_error& e)
    {
        KJB_THROW_2(IO_error, std::string(fname) + ": " + e.get_msg());
    }
}


/**
 * Reads a line off the input stream as a field with format:
 *
 * @code
 *   field name: field value
 * @endcode
 *
 * Where the separator here is a ':'.
 *
 * @param  in          Input stream to read the field line from.
 * @param  field_name  Name of the field to get the value of.
 * @param  field_buf   Buffer to read the line of input into.
 * @param  buf_len     Length of the buffer.
 * @param  separator   Character separating the field name and value. The
 *                     default is ':'.  The zero character is not allowed.
 *
 * @throw  kjb::Illegal_argument  Either the field is not the one named or
 *                                  it is not formatted properly.
 * @throw  kjb::IO_error   Could not read a line from the input stream.
 *
 * @return A constant pointer into @em field_buf where the field value is or 0 
 * if there is no value.
 *
 * @note A terminating character '\\0' is always inserted in the buffer, even 
 * if it would truncate the line.
 */
const char* Readable::read_field_value
(
    std::istream& in,
    const char*   field_name, 
    char*         field_buf,
    size_t        buf_len,
    char          separator
) 
{
    using kjb_c::kjb_strncmp;

    KJB(NTX(field_name));
    KJB(NTX(field_buf));

    in.getline(field_buf, buf_len);
    if (in.fail() || 0 == buf_len || 0 == separator) 
    {
        KJB_THROW_2(IO_error, "Could not read field line");
    }

    const char* field_ptr = field_buf;

    // Trim initial whitespace in the buffer.
    while (isspace(*field_ptr))
    {
        ++field_ptr;
    }

    // Test whether field_name is a prefix of the trimmed buffer.
    const int LFN = kjb_c::signed_strlen(field_name);
    if (! STRNCMP_EQ(field_ptr, field_name, LFN))
    {
        KJB_THROW_2(Illegal_argument, std::string("Field name '") + field_name
                                       + "' not found in '" + field_buf + "'");
    }
    field_ptr += LFN;

    // Look for the separator character; throw if not found.
    for ( ; *field_ptr != separator; ++field_ptr)
    {
        if (*field_ptr == 0)
        {
            KJB_THROW_2(Illegal_argument,
                std::string("Field '") + field_buf + "' improperly formatted");
        }
#if 1
        /*
         * New behavior, 2014 Aug 18 (predoehl).
         * We intend to require the field name to match exactly,
         * not just as a prefix.
         */
        if (!isspace(*field_ptr))
        {
            KJB_THROW_2(Illegal_argument, std::string("Field name '")
              + field_name + "' has a suffix mismatch in '" + field_buf + "'");
        }
#else
        /*
         * Old behavior (prior to Aug 2014):  do nothing.
         *
         * That would mean if field_buf="foobar: 1" and the caller scans with
         * field_name="foo" then the scan succeeds:  "bar" is ignored.
         * This seems too permissive.
         */
#endif
    }

    // Skip the separator character, and any whitespace following it.
    ++field_ptr;

    while (isspace(*field_ptr))
    {
        ++field_ptr;
    }

    if (*field_ptr == 0)
    {
        return 0;
    }

    return field_ptr;
}


/**
 * Reads a line off the input stream as a field with format:
 *
 * @code
 *   field name: field value
 * @endcode
 *
 * Where the separator here is a ':'.
 *
 * @param  in          Input stream to read the field line from.
 * @param  field_name  Name of the field to get the value of.
 * @param  separator   Character separating the field name and value. The
 *                     default is ':'.
 *
 * @throw  kjb::Illegal_argument    Either the field is not the one named or
 *                                  it is not formatted properly.
 * @throw  kjb::IO_error            Could not read a line from the input stream.
 *
 * @return A constant pointer to the field value or 0 if there is no value.
 *
 * @note The buffer for reading a field value is shared, so each time the
 * function is called, it will be overwritten.
 *
 * @note The maximum line length to read is 255.
 */
const char* Readable::read_field_value
(
    std::istream& in,
    const char*   field_name, 
    char          separator
) 
{
    // TODO This is not thread-safe.
    static char field_buf[1024] = {0};

    return read_field_value(in, field_name, field_buf, 1024, separator);
}

} // namespace kjb
