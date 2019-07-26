/* =========================================================================== *
   |
   |  Copyright (c) 1994-2010 by Kobus Barnard (author)
   |
   |  Personal and educational use of this code is granted, provided that this
   |  header is kept intact, and that the authorship is not misrepresented, that
   |  its use is acknowledged in publications, and relevant papers are cited.
   |
   |  For other use contact the author (kobus AT cs DOT arizona DOT edu).
   |
   |  Please note that the code in this file has not necessarily been adequately
   |  tested. Naturally, there is no guarantee of performance, support, or fitness
   |  for any particular task. Nonetheless, I am interested in hearing about
   |  problems that you encounter.
   |
   |  Author:  Ernesto Brau
 * =========================================================================== */

#include <gp_cpp/gp_base.h>
#include <gp_cpp/gp_mean.h>
#include <gp_cpp/gp_covariance.h>
#include <gp_cpp/gp_likelihood.h>
#include <gp_cpp/gp_normal.h>
#include <m_cpp/m_vector.h>
#include <m_cpp/m_matrix.h>
#include <iostream>
#include <string>
#include <iterator>
#include <fstream>
#include <boost/program_options.hpp>

using namespace std;
using namespace kjb;
namespace bpo = boost::program_options;

template<class OutIter>
void read_inputs(const string& fp, size_t max_count, OutIter out);

int main(int argc, char** argv)
{
    string trin_fp;
    string trout_fp;
    string tein_fp;
    double sc;
    double sg;
    double ns;
    size_t max_tr;
    size_t max_te;

    // set up options
    bpo::variables_map vm;
    bpo::options_description all_opts;
    all_opts.add_options()
        ("train-inputs,i", bpo::value<string>(&trin_fp)->required(),
            "File containing train input vectors, one per line.")
        ("train-outputs,o", bpo::value<string>(&trout_fp)->required(),
            "File containing train output values, one per line.")
        ("test-inputs,t", bpo::value<string>(&tein_fp)->required(),
            "File containing test input vectors, one per line.")
        ("scale,l", bpo::value<double>(&sc)->required(),
            "Characteristic scale parameter for the SE kernel.")
        ("svar,s", bpo::value<double>(&sg)->required(),
            "Signal variance parameter for the SE kernel.")
        ("nvar,n", bpo::value<double>(&ns)->required(),
            "Noise variance parameter for the SE kernel.")
        ("max-train", bpo::value<size_t>(&max_tr),
            "Maximum number of train examples to use.")
        ("max-test", bpo::value<size_t>(&max_te),
            "Maximum number of test examples to predict.")
        ("help,h", "Produce help message.");

    bpo::store(bpo::command_line_parser(argc, argv).options(all_opts).run(), vm);
    if(vm.count("help"))
    {
        cout << "Usage: predict_norm OPTIONS\n"
             << "Make a GP prediction on a set of test inputs"
             << "given a set of training inputs and outputs.\n"
             << all_opts << "\n" << endl;

        exit(EXIT_SUCCESS);
    }

    if(vm.count("max-train") == 0)
    {
        max_tr = numeric_limits<size_t>::max();
    }

    if(vm.count("max-test") == 0)
    {
        max_te = numeric_limits<size_t>::max();
    }

    // notify
    bpo::notify(vm);

    // read TRAIN inputs and outputs and TEST inputs
    gp::Inputs X;
    read_inputs(trin_fp, max_tr, back_inserter(X));

    gp::Inputs Xp;
    read_inputs(tein_fp, max_te, back_inserter(Xp));

    Vector y(trout_fp);
    if(y.size() > max_tr) y.resize(max_tr);

    IFT(y.size() == X.size(), Illegal_argument,
        "Train ins and outs must be same size");

    IFT(X.front().size() == Xp.front().size(), Illegal_argument,
        "Train and test inputs must be same dimension");

    // predictive distribution
    typedef gp::Predictive<gp::Zero, gp::Sqex, gp::Linear_gaussian> Pred;
    gp::Zero zero;
    gp::Sqex sqex(sc, sg);
    Pred pred = gp::make_predictive(zero, sqex, ns, X, y, Xp);
    const Vector& fp = pred.normal().get_mean();

    cout << fp << endl;
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

template<class OutIter>
void read_inputs(const string& fp, size_t max_count, OutIter out)
{
    ifstream ifs(fp);
    string line;
    size_t n = 1;
    while(getline(ifs, line) && n++ <= max_count)
    {
        stringstream ss(line);
        Vector x((istream_iterator<double>(ss)), istream_iterator<double>());
        *out++ = x;
    }
}

