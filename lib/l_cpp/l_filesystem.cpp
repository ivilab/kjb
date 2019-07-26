/* $Id: l_filesystem.cpp 15826 2013-10-21 18:24:31Z ksimek $ */

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

#include <l/l_sys_io.h>
#include <l/l_string.h>
#include <l_cpp/l_filesystem.h>
#include <l_cpp/l_exception.h>
#include <fstream>

#include <boost/format.hpp>

namespace kjb {

std::vector<std::string> file_names_from_format
(
    const std::string& name_format,
    size_t first,
    size_t num_files
)
{
    std::vector<std::string> file_names;
    bool file_exists = true;
    size_t i = first;
    while(file_exists && file_names.size() < num_files)
    {
        std::string file_name = boost::str(boost::format(name_format) % i);
        if(kjb_c::is_file(file_name.c_str()))
        {
            file_names.push_back(file_name);
        }
        else
        {
            // Be lenient if first == 0 but indices actually start from 1
            if(i > 0)
                file_exists = false;
        }
        ++i;
    }

    return file_names;
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

std::vector<std::string> dir_names_from_format
(
    const std::string& name_format,
    size_t first
)
{
    std::vector<std::string> dir_names;
    bool dir_exists = true;
    size_t i = first;
    while(dir_exists)
    {
        std::string dir_name = boost::str(boost::format(name_format) % i);
        if(kjb_c::is_directory(dir_name.c_str()))
        {
            dir_names.push_back(dir_name);
        }
        else
        {
            // Be lenient if first == 0 but indices actually start from 1
            if(i > 0)
                dir_exists = false;
        }
        ++i;
    }

    return dir_names;
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

std::vector<std::string> strings_from_format
(
    const std::string& str_format,
    size_t num_strings,
    size_t first
)
{
    std::vector<std::string> strs(num_strings);
    
    for(size_t i = 0; i < num_strings; i++)
    {
        std::string s = boost::str(boost::format(str_format) % (i + first));
        strs[i] = s;
    }

    return strs;
}


/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

std::string realpath(const std::string& path)
{
    std::vector<char> real_p(MAXPATHLEN);
    ETX(kjb_c::kjb_realpath(path.c_str(), & real_p.front(), real_p.size()));
    const int l = kjb_c::signed_strlen(& real_p.front());
    ETX(l >= MAXPATHLEN);
    return std::string(& real_p.front(), size_t(l));
}

} // namespace kjb

