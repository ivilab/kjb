#include <l/l_init.h>
#include <prob_cpp/prob_distribution.h>
#include <prob_cpp/prob_pdf.h>
#include <prob_cpp/prob_sample.h>
#include <prob_cpp/prob_conditional_distribution.h>
#include <l_cpp/l_test.h>
#include <l_cpp/l_exception.h>

using namespace std;
using namespace kjb;

#define EPSI 0.00001

int test_mv_conditional_gaussian()
{
    try
    {
        Vector mu = Vector().set(1, 2, 3, 4);
        Matrix Sigma = create_identity_matrix(4);
        Sigma(0, 0) = 30; Sigma(0, 1) = 29; Sigma(0, 2) = 25; Sigma(0, 3) = 16; 
        Sigma(1, 0) = 29; Sigma(1, 1) = 29; Sigma(1, 2) = 25; Sigma(1, 3) = 16; 
        Sigma(2, 0) = 25; Sigma(2, 1) = 25; Sigma(2, 2) = 25; Sigma(2, 3) = 16; 
        Sigma(3, 0) = 16; Sigma(3, 1) = 16; Sigma(3, 2) = 16; Sigma(3, 3) = 16; 
        MV_normal_distribution X(mu, Sigma);

        MV_normal_conditional_distribution X_2_all = X.conditional(1);

        Normal_distribution Z(0.6, sqrt(0.8));
        Vector y(3, 0.0);

        for(int i = 0; i < 1000; i++)
        {
            double z = sample(Z);
            TEST_TRUE(
                abs(pdf(Z, z) - conditional_pdf(X_2_all, Vector(1, z), y))
                < EPSI);
        }

        for(int i = 0; i < 1000; i++)
        {
            Vector x = conditional_sample(X_2_all, y);
            TEST_TRUE(abs(pdf(Z, x[0]) - conditional_pdf(X_2_all, x, y)) < EPSI);
        }
    }
    catch(const Exception& ex)
    {
        ex.print_details();
        cout << "EPIC FAIL!\n";
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

void test_mv_gaussian()
{
    size_t N = 5;
    Vector variances = create_random_vector(N);
    variances[0] = 61.717687598385318;
    variances[1] = 7.2434528489832148;
    variances[2] = 2.91482249146874;
    variances[3] = 1.766266414468693;
    variances[4] = 1.35777646694169;

    MV_gaussian_distribution dist1(Vector((int) 5, 0.0), variances);
    MV_gaussian_distribution dist2(
                    Vector((int) 5, 0.0),
                    create_diagonal_matrix(variances));

    Vector x(5);
    x[0] = 1.7525560730049601;
    x[1] = -2.026466068389996;
    x[2] = -1.9744035453971969;
    x[3] = 0.77394284555297266;
    x[4] = 1.0393176995919862;

    double lp_1 = log_pdf(dist1, x);
    double lp_2 = log_pdf(dist2, x);

    TEST_TRUE(fabs(lp_1 - lp_2) < FLT_EPSILON);

    double p_1 = pdf(dist1, x);
    double p_2 = pdf(dist2, x);

    TEST_TRUE(fabs(p_1 - p_2) < FLT_EPSILON);
}

int main(int argc, char** argv)
{
    kjb_c::kjb_init();

    int result = test_mv_conditional_gaussian();
    if(result != 0) return result;

    test_mv_gaussian();

    RETURN_VICTORIOUSLY();
}

