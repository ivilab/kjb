#include <l/l_init.h>
#include <l_cpp/l_test.h>
#include <m_cpp/m_matrix.h>
#include <m_cpp/m_vector.h>
#include <prob_cpp/prob_distribution.h>
#include <prob_cpp/prob_sample.h>
#include <prob_cpp/prob_stat.h>

using namespace std;
using namespace kjb;

int main(int argc, char** argv)
{
    kjb_c::kjb_init();

    Vector mu1, mu2, mu3;
    mu1.set(1.0, 2.0, 3.0);
    mu2.set(-1.0, 1.0, 0.0);
    mu3.set(-7.0, 0.0, 0.0, 4.0);

    Matrix Sigma1(3, 3);
    Sigma1(0, 0) = 3.9; Sigma1(0, 1) = 3.1; Sigma1(0, 2) = 1.9;
    Sigma1(1, 0) = 3.1; Sigma1(1, 1) = 3.4; Sigma1(1, 2) = 2.1;
    Sigma1(2, 0) = 1.9; Sigma1(2, 1) = 2.1; Sigma1(2, 2) = 2.2;

    Matrix Sigma2(3, 3);
    Sigma2(0, 0) = .41; Sigma2(0, 1) = .56; Sigma2(0, 2) = 0.6;
    Sigma2(1, 0) = .56; Sigma2(1, 1) = 0.8; Sigma2(1, 2) = 0.8;
    Sigma2(2, 0) = 0.6; Sigma2(2, 1) = 0.8; Sigma2(2, 2) = 0.1;

    Matrix Sigma3 = create_diagonal_matrix(Vector().set(1.0, 2.0, 3.0, 4.0));

    MV_normal_distribution N1(mu1, Sigma1);
    MV_normal_distribution N2(mu2, Sigma2);

    TEST_FALSE(N1.get_mean() == N2.get_mean());
    TEST_FALSE(N1.get_covariance_matrix() == N2.get_covariance_matrix());

    N2.set_mean(N1.get_mean());
    TEST_TRUE(N1.get_mean() == N2.get_mean());
    TEST_FALSE(N1.get_covariance_matrix() == N2.get_covariance_matrix());

    N2.set_covariance_matrix(N1.get_covariance_matrix());
    TEST_TRUE(N1.get_mean() == N2.get_mean());
    TEST_TRUE(N1.get_covariance_matrix() == N2.get_covariance_matrix());
    TEST_TRUE(fabs(pdf(N1, Vector(N1.get_dimension(), 0.0))
                - pdf(N2, Vector(N2.get_dimension(), 0.0))) < FLT_EPSILON);

    N1.set_covariance_matrix(Sigma3, mu3);
    N2.set_covariance_matrix(Sigma3, mu3);
    TEST_TRUE(N1.get_mean() == N2.get_mean());
    TEST_TRUE(N1.get_covariance_matrix() == N2.get_covariance_matrix());
    TEST_TRUE(fabs(pdf(N1, Vector(N1.get_dimension(), 0.0))
                - pdf(N2, Vector(N2.get_dimension(), 0.0))) < FLT_EPSILON);

    N1 = MV_normal_distribution(mu3, Sigma3);
    TEST_TRUE(fabs(pdf(N1, Vector(N1.get_dimension(), 1.0))
                - pdf(N2, Vector(N2.get_dimension(), 1.0))) < FLT_EPSILON);

    N1 = MV_normal_distribution(mu3, Sigma3);

    TEST_TRUE(fabs(log_pdf(N1, Vector(N1.get_dimension(), -1.0))
                - log_pdf(N2, Vector(N2.get_dimension(), -1.0))) < FLT_EPSILON);

    RETURN_VICTORIOUSLY();
}

