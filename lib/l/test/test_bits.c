/*
 * Unit test for byte-reversal functions declared in l_bits.h
 * AMP
 *
 * $Id: test_bits.c 21491 2017-07-20 13:19:02Z kobus $
 */

#include <l/l_sys_debug.h>
#include <l/l_sys_lib.h>
#include <l/l_sys_io.h>
#include <l/l_init.h>
#include <l/l_error.h>
#include <l/l_global.h>

#include <l/l_bits.h> /* functions under test */


/* Parameters are big enough to hold all test sizes. */
static int fail(int b, kjb_uint64 n_in, kjb_uint64 n_ref, kjb_uint64 n_out)
{
    /* This might not be true, but on platforms where it is false,
     * this test will have to be rewritten.
     */
    ASSERT(sizeof(kjb_uint64) == sizeof(unsigned long));

    kjb_fprintf(stderr, "Failure reversing %d-bit value:\nin = %x\n"
                        "expected output = %x\nactual output = %x.\n",
                        b, n_in, n_ref, n_out);
    return EXIT_BUG;
}


int main(void)
{
    /* Test values are byte-reversed, not bit reversed or nibble-reversed. */
    kjb_uint16 i16_fwd = 0xabcdu, i16_test = i16_fwd,
               i16_rvs = 0xcdabu;

    kjb_uint32 i32_fwd = 0xabcdef59u, i32_test = i32_fwd,
               i32_rvs = 0x59efcdabu;

    kjb_uint64 i64_fwd = 0x1234567890abcedfu, i64_test = i64_fwd,
               i64_rvs = 0xdfceab9078563412u;

    EPETE(kjb_init());

    ASSERT(i16_test != i16_rvs);
    bswap_u16(&i16_test);
    if (i16_test != i16_rvs)
    {
        return fail(16, i16_fwd, i16_rvs, i16_test);
    }

    ASSERT(i32_test != i32_rvs);
    bswap_u32(&i32_test);
    if (i32_test != i32_rvs)
    {
        return fail(32, i32_fwd, i32_rvs, i32_test);
    }

    ASSERT(i64_test != i64_rvs);
    bswap_u64(&i64_test);
    if (i64_test != i64_rvs)
    {
        return fail(64, i64_fwd, i64_rvs, i64_test);
    }

    kjb_cleanup();

    return EXIT_SUCCESS;
}
