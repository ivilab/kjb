#include <l/l_init.h>
#include <prob_cpp/prob_distribution.h>
#include <prob_cpp/prob_pdf.h>
#include <prob_cpp/prob_sample.h>
#include <prob_cpp/prob_conditional_distribution.h>
#include <l_cpp/l_test.h>

using namespace std;
using namespace kjb;

#define EPSI 0.00001

int main(int argc, char** argv)
{
    kjb_c::kjb_init();

    Normal_distribution X(2, 0.5);
    Normal_conditional_distribution ZlY(Normal_on_normal_dependence(0.5));
    double y = 2;

    for(int i = 0; i < 1000; i++)
    {
        double x = sample(X);
        TEST_TRUE(abs(pdf(X, x) - conditional_pdf(ZlY, x, y)) < EPSI);
    }

    for(int i = 0; i < 1000; i++)
    {
        double z = conditional_sample(ZlY, y);
        TEST_TRUE(abs(pdf(X, z) - conditional_pdf(ZlY, z, y)) < EPSI);
    }

    RETURN_VICTORIOUSLY();
}

