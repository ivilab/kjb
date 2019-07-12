/* $Id: test_readable.cpp 21356 2017-03-30 05:34:45Z kobus $ */

#include <l/l_sys_lib.h>
#include <l/l_sys_io.h>
#include <l/l_init.h>
#include <l/l_debug.h>
#include <l_cpp/l_stdio_wrap.h>
#include <l_cpp/l_util.h>
#include <l_cpp/l_test.h>
#include <l_cpp/l_readable.h>

#include <string>
#include <sstream>
#include <iostream>

namespace
{

const std::string F1 = "Description",    // field names
                  F2 = "Storage",
                  F3 = "Calories",

                  D1 = "This dessert is called ", // human-readable gloss
                  D2 = "\n" + F2 + " requirements: ",
                  D3 = "\n" + F3 + ": ";


class Dessert : public kjb::Readable
{
    std::string description_, storage_;
    float calories_;

public:

    // stream putter
    inline
    friend std::ostream& operator<<(std::ostream& s, const Dessert& d)
    {
        s   << D1 << d.description_
            << D2 << d.storage_
            << D3 << d.calories_ << '\n';
        return s;
    }

    using Readable::read;  // base version can accept a filename

    // read object state from a stream (required by abstract base class)
    void read(std::istream& s)
    {
        const size_t BUF_SZ = 4096;
        char buf[BUF_SZ];
        *buf = 0;

        description_ = read_field_value(s, F1.c_str(), buf, BUF_SZ);
        storage_ = read_field_value(s, F2.c_str(), buf, BUF_SZ);
        calories_ = atof(read_field_value(s, F3.c_str(), buf, BUF_SZ));
    }
};


} // end anon ns


int main(int argc, char** argv)
{
    KJB(EPETE(kjb_c::kjb_init()));
    bool happy = false;

    // --------------------------------------------------------------------


    // Test ordinary, proper behavior

    const std::string V1 = "apple pie", V2 = "room temperature", V3 = "278.5";

    // Generate a file representing a Dessert.
    // In it, we fiddle with the whitespace because Readable should ignore it.
    kjb::Temporary_File f;
    kjb_c::kjb_fputs(f, ( F1 + ":" + V1 + "\n   "
                        + F2 + "  :   " + V2 + "\n"
                        + F3 + ": " + V3).c_str());
    fflush(f);

    if (kjb_c::is_interactive())
    {
        kjb_c::p_stderr("Test file f contains:\n------------------\n");
        kjb_c::kjb_system(("cat " + f.get_filename()).c_str());
        kjb_c::kjb_putc('\n');
    }

    // Read the file using the Readable member function.
    Dessert d;
    d.read(f.get_filename().c_str());
    f.close();

    // Make sure the results are correct.
    std::ostringstream t;
    t << d; // observe above that operator<<() adds a newline to the end.
    const std::string EXPECT = D1 + V1 + D2 + V2 + D3 + V3 + "\n";
    happy = (EXPECT == t.str());

    if (kjb_c::is_interactive())
    {
        if (!happy)
        {
            std::cout << "expecting:\n" << EXPECT
                << "\n(length " << EXPECT.size()
                << ")\nreceived:\n" << t.str()
                << "\n(length " << t.str().size()
                << ")\n";
        }
        else
        {
            std::cout << "As expected, result was this:\n" << EXPECT << '\n';
        }
    }
    TEST_TRUE(happy);


    // --------------------------------------------------------------------


    // Generate a bad file representing a Desert.
    kjb::Temporary_File g;
    const std::string F4 = "Cactus";
    kjb_c::kjb_fputs(g,
            (F1 + ": Sonora\n" + F2 + ": plenty\n" + F4 + ": yes\n").c_str());
    fflush(g);
    happy = false;

    if (kjb_c::is_interactive())
    {
        kjb_c::pso("Test file g contains:\n------------------\n");
        kjb_c::kjb_system(("cat " + g.get_filename()).c_str());
        kjb_c::kjb_putc('\n');
    }

    try
    {
        Dessert e;
        e.read(g.get_filename().c_str());
    }
    catch (const kjb::Illegal_argument& e)
    {
        if (kjb_c::is_interactive()) e.print_details();
        const std::string MSG = "Field name '" + F3 + "' not found in '" + F4;
        happy = 0==e.get_msg().find(MSG);
    }
    g.close();
    TEST_TRUE(happy);


    // --------------------------------------------------------------------


    // Test new behavior (field_name must match exactly, not just a prefix)
    kjb::Temporary_File h;
    const std::string BAD_SUFFIX = "z: yes";
    kjb_c::kjb_fputs(h, ( F1 + " : ice cream\n  "
                        + F2 + "  :  frozen\n"
                        + F3 + BAD_SUFFIX + "\n").c_str());
    fflush(h);
    happy = false;

    if (kjb_c::is_interactive())
    {
        kjb_c::p_stderr("Test file h contains:\n------------------\n");
        kjb_c::kjb_system(("cat " + h.get_filename()).c_str());
        kjb_c::kjb_putc('\n');
    }

    try
    {
        Dessert e;
        e.read(h.get_filename().c_str());
    }
    catch (const kjb::Illegal_argument& e)
    {
        if (kjb_c::is_interactive()) e.print_details();
        const std::string MSG = "Field name '" + F3
                              + "' has a suffix mismatch in '"
                              + F3 + BAD_SUFFIX + "'";
        happy = 0==e.get_msg().find(MSG);
    }
    h.close();
    TEST_TRUE(happy);

    // --------------------------------------------------------------------


    kjb_c::kjb_cleanup();
    RETURN_VICTORIOUSLY();
}

