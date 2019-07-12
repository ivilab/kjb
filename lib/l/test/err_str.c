/**
 * @file
 * @author Andrew Predoehl
 */

/* $Id: err_str.c 21596 2017-07-30 23:33:36Z kobus $
 */

#include "l/l_incl.h"

#define FAIL()  fail( __LINE__ )

#define TENDIGITS "0123456789"

#define TENSZ 10

#define BUFSZ 30 

static int fail( int line )
{
    kjb_fprintf( stderr, "failure on line %d\n", line );
    return EXIT_BUG;
}

int main (void)
{
    int count;
    const char *ten = TENDIGITS;
    char buf[ BUFSZ ];

    ASSERT( TENSZ == signed_strlen( TENDIGITS ) );
    ASSERT( BUFSZ > 2 * ( 1 + TENSZ ) );

    EPETE( kjb_init() );

    set_error( ten );

    count = kjb_get_strlen_error();

    if ( count != TENSZ )
        return FAIL();

    kjb_get_error( buf, BUFSZ );
    if ( ! STRNCMP_EQ( buf, ten, BUFSZ ) )
        return FAIL();

    count = kjb_get_strlen_error();

    if ( count != 0 )
        return FAIL();

    kjb_get_error( buf, BUFSZ );

    if ( buf[ 0 ] != 0 )
        return FAIL();

    set_error( ten );
    add_error( ten );
    count = kjb_get_strlen_error();

    if ( count != 2 * TENSZ + 1 )
        return FAIL();

    kjb_get_error( buf, BUFSZ );

    if ( ! STRNCMP_EQ( buf, TENDIGITS "\n" TENDIGITS , BUFSZ ) )
        return FAIL();

    kjb_cleanup();

    return EXIT_SUCCESS;
}
