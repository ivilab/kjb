#include <gp_cpp/gp_base.h>
#include <gp_cpp/gp_mean.h>
#include <gp_cpp/gp_covariance.h>
#include <gp_cpp/gp_predictive.h>
#include <gp_cpp/gp_normal.h>
#include <gp_cpp/gp_likelihood.h>
#include <gp_cpp/gp_prior.h>
#include <gp_cpp/gp_sample.h>
#include <gp_cpp/gp_pdf.h>
#include <prob_cpp/prob_distribution.h>
#include <prob_cpp/prob_pdf.h>
#include <prob_cpp/prob_sample.h>
#include <l_cpp/l_test.h>
#include <m_cpp/m_vector.h>
#include <vector>
#include <algorithm>
#include <functional>

using namespace std;
using namespace kjb;

typedef MV_normal_distribution Mvn;
typedef gp::Predictive_nl<gp::Constant, gp::Sqex> Pred_nl;
typedef gp::Predictive<gp::Constant, gp::Sqex, gp::Linear_gaussian> Pred;

bool compare_normals(const Mvn& N1, const Mvn& N2)
{
    double md = max_abs_difference(N1.get_mean(), N2.get_mean());
    double cd = max_abs_difference(
                        N1.get_covariance_matrix(),
                        N2.get_covariance_matrix());

    return md < FLT_EPSILON && cd < FLT_EPSILON;
}

int main(int, char**)
{
    const size_t N = 100;

    gp::Constant zero(0.0);
    gp::Constant ten(10.0);
    gp::Squared_exponential sqex(1.0, 1.0);

    // create inputs
    gp::Inputs X1(N);
    gp::Inputs X2(N);
    gp::Inputs W1(N);
    gp::Inputs W2(N);
    for(size_t n = 0; n < N; n++)
    {
        X1[n].set(n);
        W1[n].set(n + 0.5);
        X2[n].set(N + n);
        W2[n].set(N + n + 0.5);
    }

    // create outputs
    Vector f1(X1.size());
    Vector f2(X2.size());
    for(size_t i = 0; i < f1.size(); i++)
    {
        f1[i] = sin(X1[i][0]);
        f2[i] = sin(X2[i][0]);
    }

    ////////// TEST CACHING IN PRIOR //////////
    gp::Prior<gp::Constant, gp::Sqex> prior1(zero, sqex, X1.begin(), X1.end());
    gp::Prior<gp::Constant, gp::Sqex> prior2(zero, sqex, X2.begin(), X2.end());
    gp::Prior<gp::Constant, gp::Sqex> prior3(ten, sqex, X2.begin(), X2.end());

    // test assignment
    gp::Prior<gp::Constant, gp::Sqex> prior = prior1;
    TEST_TRUE(compare_normals(prior.normal(), prior1.normal()));

    // test setting inputs
    prior.set_inputs(X2.begin(), X2.end());
    TEST_TRUE(compare_normals(prior.normal(), prior2.normal()));

    // test setting MF
    prior.set_mean_function(ten);
    TEST_TRUE(compare_normals(prior.normal(), prior3.normal()));

    ////////// TEST NL PREDCTIVE CACHING //////////
    Pred_nl pred1 = gp::make_predictive_nl(zero, sqex, X1, f1, W1);
    Pred_nl pred2 = gp::make_predictive_nl(zero, sqex, X1, f2, W1);
    Pred_nl pred3 = gp::make_predictive_nl(zero, sqex, X1, f1, W2);
    Pred_nl pred4 = gp::make_predictive_nl(zero, sqex, X1, f2, W2);
    Pred_nl pred5 = gp::make_predictive_nl(zero, sqex, X2, f1, W1);
    Pred_nl pred6 = gp::make_predictive_nl(zero, sqex, X2, f2, W2);
    Pred_nl pred7 = gp::make_predictive_nl(ten, sqex, X2, f2, W2);

    // test assignment
    Pred_nl pred = pred1;
    TEST_TRUE(compare_normals(pred.normal(), pred1.normal()));

    // test against pred1
    pred.set_train_inputs(X1.begin(), X1.end());
    TEST_TRUE(compare_normals(pred.normal(), pred1.normal()));

    // test against pred2
    pred.set_train_outputs(f2.begin(), f2.end());
    TEST_TRUE(compare_normals(pred.normal(), pred2.normal()));

    // test against pred3
    pred.set_train_outputs(f1.begin(), f1.end());
    pred.set_test_inputs(W2.begin(), W2.end());
    TEST_TRUE(compare_normals(pred.normal(), pred3.normal()));

    // test against pred4
    pred.set_train_outputs(f2.begin(), f2.end());
    TEST_TRUE(compare_normals(pred.normal(), pred4.normal()));

    // test against pred5
    pred.set_train_inputs(X2.begin(), X2.end());
    pred.set_train_outputs(f1.begin(), f1.end());
    pred.set_test_inputs(W1.begin(), W1.end());
    TEST_TRUE(compare_normals(pred.normal(), pred5.normal()));

    // test against pred6
    pred.set_train_outputs(f2.begin(), f2.end());
    pred.set_test_inputs(W2.begin(), W2.end());
    TEST_TRUE(compare_normals(pred.normal(), pred6.normal()));

    // test against pred7
    pred.set_mean_function(ten);
    TEST_TRUE(compare_normals(pred.normal(), pred7.normal()));

    ////////// TEST NORMAL PREDCTIVE CACHING //////////
    double sg1 = 0.1;
    double sg2 = 0.01;
    Pred npred1 = gp::make_predictive(zero, sqex, sg1, X1, f1, W1);
    Pred npred2 = gp::make_predictive(zero, sqex, sg1, X1, f2, W1);
    Pred npred3 = gp::make_predictive(zero, sqex, sg1, X1, f1, W2);
    Pred npred4 = gp::make_predictive(zero, sqex, sg1, X1, f2, W2);
    Pred npred5 = gp::make_predictive(zero, sqex, sg2, X2, f1, W1);
    Pred npred6 = gp::make_predictive(zero, sqex, sg2, X2, f2, W2);
    Pred npred7 = gp::make_predictive(ten, sqex, sg2, X2, f2, W2);
    Pred npred8 = gp::make_predictive(ten, sqex, sg1, X2, f2, W2);

    // test assignment
    Pred npred = npred1;
    TEST_TRUE(compare_normals(npred.normal(), npred1.normal()));

    // test against npred1
    npred.set_train_inputs(X1.begin(), X1.end());
    TEST_TRUE(compare_normals(npred.normal(), npred1.normal()));

    // test against npred2
    npred.set_train_outputs(f2.begin(), f2.end());
    TEST_TRUE(compare_normals(npred.normal(), npred2.normal()));

    // test against npred3
    npred.set_train_outputs(f1.begin(), f1.end());
    npred.set_test_inputs(W2.begin(), W2.end());
    TEST_TRUE(compare_normals(npred.normal(), npred3.normal()));

    // test against npred4
    npred.set_train_outputs(f2.begin(), f2.end());
    TEST_TRUE(compare_normals(npred.normal(), npred4.normal()));

    // test against npred5
    npred.set_train_inputs(X2.begin(), X2.end());
    npred.set_train_outputs(f1.begin(), f1.end());
    npred.set_test_inputs(W1.begin(), W1.end());
    npred.set_likelihood(gp::Linear_gaussian(sg2, N));
    TEST_TRUE(compare_normals(npred.normal(), npred5.normal()));

    // test against npred6
    npred.set_train_outputs(f2.begin(), f2.end());
    npred.set_test_inputs(W2.begin(), W2.end());
    TEST_TRUE(compare_normals(npred.normal(), npred6.normal()));

    // test against npred7
    npred.set_mean_function(ten);
    TEST_TRUE(compare_normals(npred.normal(), npred7.normal()));

    // test against npred8
    npred.set_likelihood(gp::Linear_gaussian(sg1, N));
    TEST_TRUE(compare_normals(npred.normal(), npred8.normal()));

    RETURN_VICTORIOUSLY();
}

