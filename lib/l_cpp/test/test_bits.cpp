/**
 * @file
 * @brief unit test for lib/l_cpp/l_bits.h (which does endian-flips)
 * @author andrew predoehl
 */
/*
 * $Id: test_bits.cpp 21755 2017-09-07 21:54:51Z kobus $
 */

#include "l/l_init.h"
#include "l/l_sys_io.h"
#include "l/l_sys_lib.h"
#include "l/l_sys_debug.h"
#include "l_cpp/l_cpp_bits.h"
#include "l_cpp/l_util.h"

#include <algorithm>

#define FRF(x) do { if (EXIT_FAILURE==(x)) return EXIT_FAILURE; } while(0)

namespace {

const char *s = "fistular", *t = "ralutsif";
size_t SMAX = strlen(s);


template <typename U>
int test_swapping(U& ux)
{
    const size_t usize = sizeof(ux.a);
    const char *answer = t + SMAX - usize;

    std::copy(s, s+usize, ux.a);
    //std::string qf(ux.a, usize);
    kjb::swap_bytes(ux.x);
    //std::string qr(ux.a, usize);

    //std::cout << "Input: '" << qf << "'; output: '" << qr << "' -- wow.\n";
    if (0 == kjb_c::kjb_strncmp(ux.a, answer, usize)) return EXIT_SUCCESS;

    kjb_c::pso("Error for input of size %u\n", usize);
    return EXIT_FAILURE;
}


union U2 { char a[2]; kjb_c::kjb_uint16 x; } u2;
union U4 { char a[4]; kjb_c::kjb_uint32 x; } u4;
union U8 { char a[8]; kjb_c::kjb_uint64 x; } u8;

}

int main()
{
    kjb_c::kjb_init();

    ASSERT(2 == sizeof(U2));
    ASSERT(4 == sizeof(U4));
    ASSERT(8 == sizeof(U8));

    FRF(test_swapping(u2));
    FRF(test_swapping(u4));
    FRF(test_swapping(u8));

    kjb_c::kjb_cleanup();

    return EXIT_SUCCESS;
}

