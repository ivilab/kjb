
/* $Id: l_config.c 21520 2017-07-22 15:09:04Z kobus $ */

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
* =========================================================================== */

#include "l/l_gen.h"     /* Only safe as first include in a ".c" file. */
#include "l/l_io.h"
#include "l/l_string.h"
#include "l/l_sys_scan.h"
#include "l/l_parse.h"
#include "l/l_verbose.h"
#include "l/l_config.h"


#ifdef __cplusplus
extern "C" {
#endif

/* -------------------------------------------------------------------------- */

#define MAX_NUM_CONFIG_FILES  1000

#ifndef PROGRAMMERS_LOGIN
#    ifdef PROGRAMMER
#        define PROGRAMMERS_LOGIN PROGRAMMER
#    endif
#endif

#ifndef SHARED_LOGIN
#    define SHARED_LOGIN      "kobus"
#endif

/* -------------------------------------------------------------------------- */

static int get_config_directories
(
    char        directories[ MAX_NUM_CONFIG_FILES ][ MAX_FILE_NAME_SIZE ],
    const char* sub_dir
);

static FILE* open_config_file_2
(
    const char* env_var,
    int         num_files,
    char        file_name_array[ MAX_NUM_CONFIG_FILES ][ MAX_FILE_NAME_SIZE ],
    const char* message_name
);

/* -------------------------------------------------------------------------- */

static const char* local_dirs[] =
{
    "work",
#ifdef RISK_NFS_WAIT
    "net/v04/work",
#endif 
    "data",
#ifdef RISK_NFS_WAIT
    "net/v05/data",
#endif 
    "space",
#ifdef RISK_NFS_WAIT
    "net/v05/space",
#endif 
    NULL
};

/* -------------------------------------------------------------------------- */

/* =============================================================================
 *                             check_config_sub_dir
 *
 * Checks validity of a config sub dir
 *
 * Checks that there is a configuration sub-directory in one of the places which
 * will be searched if this sub-directory is later used as part of a
 * configuration file specification.
 *
 * Returns:
 *     Either NO_ERROR if the sub dir is valid, and ERROR if it is not.
 *
 * Index: configuration files, I/O
 *
 * -----------------------------------------------------------------------------
*/

int check_config_sub_dir(const char* sub_dir)
{
    char directories[ MAX_NUM_CONFIG_FILES ][ MAX_FILE_NAME_SIZE ];
    int  num_dirs;
    int count;


    ERE(num_dirs = get_config_directories(directories, sub_dir));

    for (count = 0; count < num_dirs; count++)
    {
        if (    (directories[ count ][ 0 ] != '\0')
             && (get_path_type(directories[ count ]) == PATH_IS_DIRECTORY)
           )
        {
            return NO_ERROR;
        }
    }

    set_error("%q is not a valid configuration sub-directory.\n",
              sub_dir);

    return ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                        read_dbl_from_config_file
 *
 * Reads a number from a config file
 *
 * This routine reads a number from a configuration file, checking several
 * possible locations for the file. If the parameter "env_var" is not NULL, then
 * it first tries using this string as the configuration file name.  Next it
 * looks for "file_name" in the current directory, then the user's home
 * directory, and, depending on how the library was built, then a "shared"
 * home directory and/or the programmer's home directory, and/or other
 * locations. See open_config_file for more details on the search locations. 
 *
 * If the parameter sub_dir is not null, then it is used as a subdirectory for
 * all search locations other than one in the env_var variable (if not NULL),
 * and the current directory. 
 *
 * The parameter message_name can be used to specify a name for the
 * configuration file to be used in error and verbose output messages. If
 * message_name is NULL, then "configuration" is used.
 *
 * If the buffer config_file_name is not NULL, then the file name actually used
 * is copied into it. If it is used, then its size must be passed in via
 * config_file_name_size;
 *
 * If file_name is the special name "off" (or "none" or "0"), then the above
 * does not apply. In this case, *value_ptr is set to DBL_NOT_SET, which is
 * guaranteed to be negative.
 *
 * The file name actually used is put into the buffer config_file_name whose
 * size must be passed in via max_len;
 *
 * The result is put into *value_ptr.
 *
 * Returns:
 *     Either NO_ERROR, or ERROR, with an appropriate error message being set.
 *
 * Index: configuration files, I/O
 *
 * -----------------------------------------------------------------------------
*/

int read_dbl_from_config_file(double* value_ptr, const char* env_var,
                               const char* sub_dir, const char* file_name,
                               const char* message_name, char* config_file_name, size_t config_file_name_size)
{
    FILE* fp;
    char  temp_config_file_name[ MAX_FILE_NAME_SIZE ];
    char  line[ 100 ];
    char* line_pos;
    int   result;


    if (is_no_value_word(file_name))
    {
        *value_ptr = DBL_NOT_SET;
        config_file_name[ 0 ] = '\0';
        return NO_ERROR;
    }

    NRE(fp = open_config_file(env_var, sub_dir, file_name, message_name));
    BUFF_GET_USER_FD_NAME(fileno(fp), temp_config_file_name);
    BUFF_GET_REAL_LINE(fp, line);
    ERE(kjb_fclose(fp));

    line_pos = line;
    trim_beg(&line_pos);
    trim_end(line_pos);

    result = ss1d(line_pos, value_ptr);

    if (result == ERROR)
    {
        insert_error("Error reading value from config file %s.",
                     temp_config_file_name);
    }
    else if (config_file_name != NULL)
    {
        kjb_strncpy(config_file_name, temp_config_file_name,
                    config_file_name_size);
    }

    return result;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                          open_config_file
 *
 * Opens a configuration file
 *
 * This routine opens a configuration file, checking several possible locations
 * for the file. If the parameter "env_var" is not NULL, then it first tries
 * using this string as the configuration file name. Next it looks for
 * "file_name" in the current directory. Then it looks in the user's home
 * directory, and the a number of other locations (e.g., /work, /data, /space)
 * using the user id as a subdirectory. .  Then it does the same based on a
 * compiled in shared login id (if different), and then it does the same for the
 * programmers login (if different from the preceeding two).  If the sub_dir
 * argument is not NULL, then it is used as a sub-directory in all paths except
 * the in the env_var parameter and the current directory.
 *
 * Example:
 *     Suppose sub_dir is "x" and file_name is "y".  Suppose the user is
 *     "alice". Also supposed that the shared directory is "~vision" (compiled
 *     in constant) and the programmer's directory is "~kobus" (another compiled
 *     in constant). Finally, suppose that /work, /data, and /space are the
 *     current compiled in locations for user space that might contain
 *     configuration files. Then the following files are tried in the order
 *     listed.
 * |            ./y
 * |            ~/x/y
 * |            /work/alice/x/y
 * |            /data/alice/x/y
 * |            /space/alice/x/y
 * |            ~vision/x/y
 * |            /work/vision/x/y
 * |            /data/vision/x/y
 * |            /space/vision/x/y
 * |            ~kobus/x/y
 * |            /work/kobus/x/y
 * |            /data/kobus/x/y
 * |            /space/kobus/x/y
 *
 * The paramter message_name can be used to specify a name for the configuration
 * file to be used in composing an error message. Otherwise "configuration" is
 * used.
 *
 * Note:
 *     This routine looks in a lot of places. It is not clear if looking in
 *     /work, etc., is helpful. It was developed in a different environment. A
 *     lot of the issues have changed. Hence it is possible that the directories
 *     will change.
 *
 * Returns:
 *     Either NO_ERROR, or ERROR, with an appropriate error message being set.
 *
 * Index: configuration files, I/O
 *
 * -----------------------------------------------------------------------------
*/

FILE* open_config_file(const char* env_var, const char* sub_dir,
                       const char* file_name, const char* message_name)
{
    char directories[ MAX_NUM_CONFIG_FILES ][ MAX_FILE_NAME_SIZE ];
    char paths[ MAX_NUM_CONFIG_FILES ][ MAX_FILE_NAME_SIZE ];
    int  num_dirs;
    int count;


    ERN(num_dirs = get_config_directories(directories, sub_dir));

    for (count = 0; count < num_dirs; count++)
    {
        paths[ count ][ 0 ] = '\0';

        if (directories[ count ][ 0 ] != '\0')
        {
            BUFF_CPY(paths[ count ], directories[ count ]);
            BUFF_CAT(paths[ count ], DIR_STR);
        }

        BUFF_CAT(paths[ count ], file_name);
    }

    return open_config_file_2(env_var, num_dirs, paths, message_name);
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

static int get_config_directories(char directories[ MAX_FILE_NAME_SIZE ][ MAX_FILE_NAME_SIZE ],
                                  const char* sub_dir)
{
    static const char* users[ ] =
    {
        ""
#ifdef SHARED_LOGIN
#ifdef HOME_STR
        , SHARED_LOGIN
#endif
#endif
#ifdef PROGRAMMERS_LOGIN
#ifdef HOME_STR
        , PROGRAMMERS_LOGIN
#endif
#endif
    };
    const int num_users = sizeof(users) / sizeof(users[ 0 ]);
    char user_id[ 100 ];
    int  count = 0;
    int  user_num;


    /* This puts in what is needed for the current directory (nothing!). */
    directories[ count ][ 0 ] = '\0';
    count++;

    ERE(BUFF_GET_USER_ID(user_id));

    for (user_num = 0; user_num < num_users; user_num++)
    {
        int local_dir_num = 0;
        Bool duplicate_user = FALSE;
        int visited; 
        const char* cur_user_id = user_num ? users[ user_num ] : user_id; 

        if (count >= MAX_NUM_CONFIG_FILES)
        {
            SET_BUFFER_OVERFLOW_BUG();
            return ERROR;
        }

        for (visited = 0; visited< user_num; visited++) 
        {
            const char* user_visited = visited ? users[visited] : user_id;

            if (STRCMP_EQ(user_visited, cur_user_id))
            {
                 duplicate_user = TRUE;
                 break;
            }
        }

        if (duplicate_user)
        {
            continue;
        }

        BUFF_CPY(directories[ count ], HOME_STR);
        BUFF_CAT(directories[ count ], users[ user_num ]);

        if ((sub_dir != NULL) && (*sub_dir != '\0'))
        {
            BUFF_CAT(directories[ count ], DIR_STR);
            BUFF_CAT(directories[ count ], sub_dir);
        }

        count++;

        while (local_dirs[ local_dir_num ] != NULL)
        {
            if (count >= MAX_NUM_CONFIG_FILES)
            {
                SET_BUFFER_OVERFLOW_BUG();
                return ERROR;
            }

            directories[ count ][ 0 ] = '\0';
            BUFF_CAT(directories[ count ], DIR_STR);
            BUFF_CAT(directories[ count ], local_dirs[ local_dir_num ]);

            BUFF_CAT(directories[ count ], DIR_STR);
            BUFF_CAT(directories[ count ], cur_user_id);

            if (sub_dir != NULL)
            {
                BUFF_CAT(directories[ count ], DIR_STR);
                BUFF_CAT(directories[ count ], sub_dir);
            }

            count++;
            local_dir_num++;
        }
    }

    return count;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

static FILE* open_config_file_2(const char* env_var, int num_files,
                                char file_name_array[ MAX_NUM_CONFIG_FILES][ MAX_FILE_NAME_SIZE ],
                                const char* message_name)
{
    FILE* fp = NULL;
    char  env_var_file_name[ MAX_FILE_NAME_SIZE ];
    int   count;


    if ((env_var != NULL) && (*env_var != '\0'))
    {
        verbose_pso(15, "Checking ENVIRONMENT variable %s for %s file.\n",
                    env_var, (message_name == NULL) ? "configuration"
                                                    : message_name);

        if (BUFF_GET_ENV(env_var, env_var_file_name) != ERROR)
        {
            fp = kjb_fopen(env_var_file_name, "r");
        }
    }

    if ((fp == NULL) && (file_name_array != NULL))
    {
        for (count = 0; count < num_files; count++)
        {
            const char* file_name = file_name_array[ count ];

            verbose_pso(15, "Trying %s for %s file.\n", file_name,
                        (message_name == NULL) ? "configuration"
                                               : message_name);
            fp = kjb_fopen(file_name, "r");

            if (fp != NULL) break;
        }
    }

    if (fp == NULL)
    {
        set_error("Unable to find %s file.",
                  (message_name == NULL) ? "configuration" : message_name);

        if ((env_var != NULL) && (*env_var != '\0'))
        {
            add_error("Checked for file in path in environment variable %q.",
                      env_var);
        }

        if ((fp == NULL) && (file_name_array != NULL))
        {
            for (count = 0; count < num_files; count++)
            {
                const char* file_name = file_name_array[ count ];

                add_error("Checked for file \"%s\".", file_name);
            }
        }
    }
    else
    {
        verbose_pso(10, "Using %F for %s file.\n", fp,
                    (message_name == NULL) ? "configuration" : message_name);
    }

   return fp;
  }

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                          get_config_file
 *
 * Finds a configuration file
 *
 * This routine implements essentially a "dry run" of open_config_file(). If a
 * config file is found, its name is put into the buffer "config_file", whose
 * size must be passed in via "config_file_name_size". The file can be
 * subsequently opened for use at some later point. See open_config_file() for
 * details on the search locations used. 
 *
 * The file name actually used is put into the buffer config_file_name whose
 * size must be passed in via config_file_name_size;
 *
 * The paramter message_name can be used to specify a name for the configuration
 * file to be used in composing an error message. Otherwise "configuration" is
 * used.
 *
 * Returns:
 *     Either NO_ERROR, or ERROR, with an appropriate error message being set.
 *
 * Index: configuration files, I/O
 *
 * -----------------------------------------------------------------------------
*/

int get_config_file(const char* env_var, const char* sub_dir,
                    const char* file_name, const char* message_name,
                    char* config_file, size_t config_file_size)
{
    FILE* fp;


    NRE(fp = open_config_file(env_var, sub_dir, file_name, message_name));

    get_user_fd_name(fileno(fp), config_file, config_file_size);

    ERE(kjb_fclose(fp));

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#ifdef __cplusplus
}
#endif

