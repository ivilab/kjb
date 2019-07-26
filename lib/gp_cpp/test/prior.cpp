#include <gp_cpp/gp_base.h>
#include <gp_cpp/gp_mean.h>
#include <gp_cpp/gp_covariance.h>
#include <gp_cpp/gp_prior.h>
#include <prob_cpp/prob_distribution.h>
#include <m_cpp/m_vector.h>
#include <m_cpp/m_matrix.h>
#include <l_cpp/l_test.h>

using namespace std;
using namespace kjb;

int main(int, char**)
{
    const size_t N = 100;

    // create prior
    gp::Zero zero;
    gp::Squared_exponential sqex(1.0, 1.0);

    gp::Inputs X(N);
    for(size_t n = 0; n < N; n++)
    {
        X[n].set(n);
    }

    gp::Prior<gp::Zero, gp::Sqex> prior(zero, sqex, X.begin(), X.end());
    const MV_normal_distribution& P = prior.normal();

    // test mean is zero
    TEST_TRUE(P.get_mean() == Vector((int)N, 0.0));

    // test covariance matrix is symmetric
    const Matrix& S = P.get_covariance_matrix();
    const Matrix& S_t = matrix_transpose(S);
    TEST_TRUE(S == S_t);

    // create another prior
    Vector mu = sample(P);
    for(size_t n = 0; n < N; n++)
    {
        X[n].set(n + 1);
    }

    gp::Manual m1(X, mu);
    gp::Prior<gp::Manual, gp::Sqex> prior2(m1, sqex, X.begin(), X.end());
    const MV_normal_distribution& P2 = prior2.normal();

    // test mean is mu
    TEST_TRUE(P2.get_mean() == mu);

    RETURN_VICTORIOUSLY();
}

