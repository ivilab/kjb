/* $Id: l_filesystem.h 15826 2013-10-21 18:24:31Z ksimek $ */

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
|     Ernesto Brau
|
* =========================================================================== */


#ifndef L_FILESYSTEM_H
#define L_FILESYSTEM_H

#include <string>
#include <vector>
#include <limits>

namespace kjb {

/** @brief  Expands the format into a set of (existing) file names. */
std::vector<std::string> file_names_from_format
(
    const std::string& name_format,
    size_t first = 1,
    size_t num_files = std::numeric_limits<size_t>::max()
);

/** @brief  Expands the format into a set of (existing) file names. */
std::vector<std::string> dir_names_from_format
(
    const std::string& name_format,
    size_t first = 1
);

/** @brief  Expands the format into a set strings. */
std::vector<std::string> strings_from_format
(
    const std::string& str_format,
    size_t num_strings,
    size_t first = 1
);


/**
 * Return the extension of a filename, with the '.' removed
 */
inline std::string get_extension(const std::string& fname)
{
    return fname.substr(fname.find_last_of(".") + 1);
}


/// @brief return a canonicalized path, by wrapping kjb_c::kjb_realpath().
std::string realpath(const std::string& path);


} //namespace kjb

#endif

