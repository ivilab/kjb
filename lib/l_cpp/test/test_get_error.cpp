/*
 * $Id: test_get_error.cpp 19990 2015-10-29 16:06:00Z predoehl $
 */
#include <l/l_sys_std.h>
#include <l_cpp/l_exception.h>
#include <l_cpp/l_test.h>

int main(int, char**)
{
    const std::string gr="A screaming comes across the sky.";
    kjb_c::set_error(gr.c_str());
    std::string x = kjb::kjb_get_error();
    TEST_TRUE(x == gr);

    kjb_c::kjb_clear_error();
    x = kjb::kjb_get_error();
    TEST_TRUE(x.empty());

    const std::string pp="It is a truth universally acknowledged that...";
    kjb_c::add_error(pp.c_str());
    x = kjb::kjb_get_error();
    TEST_TRUE(x == pp);

    RETURN_VICTORIOUSLY();
}

