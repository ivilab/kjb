#include <l_cpp/l_test.h>
#include <prob_cpp/prob_distribution.h>
#include <prob_cpp/prob_sample.h>
#include <prob_cpp/prob_histogram.h>

#include <iostream>
#include <iterator>
#include <vector>
#include <utility>

using namespace kjb;
using namespace std;

int main(int argc, char** argv)
{
    Vector m(1.0, 2.0); 
    Matrix s(2, 2);
    s(0, 0) = 1.0; s(0, 1) = 0.5;
    s(1, 0) = 0.5; s(1, 1) = 1.0;
    MV_normal_distribution mnd(m, s);
    vector<Vector> samples(1000);
    for(size_t i = 0; i < samples.size(); i++)
    {
        samples[i] = sample(mnd);
    }
    //copy(samples.begin(), samples.end(), ostream_iterator<Vector>(cout, "\n"));
    //cout << endl << endl;
    const size_t num_bins = 10;
    pair<double, double> ranges; 

    Vector left = *std::min_element(samples.begin(), samples.end(),
                                    Index_less_than<Vector>(0));
    Vector right = *std::max_element(samples.begin(), samples.end(),
                                     Index_less_than<Vector>(0));
    Vector bottom = *std::min_element(samples.begin(), samples.end(),
                                    Index_less_than<Vector>(1));
    Vector top = *std::max_element(samples.begin(), samples.end(),
                                     Index_less_than<Vector>(1));
                                    
    ranges = make_pair(left[0], right[0]);
   
    Histogram_2d histogram(samples.begin(), samples.end(), 
                           num_bins, num_bins, 
                           ranges, ranges);

    const Matrix& hist_mat = histogram.as_matrix();

    Matrix n_hist = histogram.normalized();
    double sum = sum_matrix_cols(n_hist).sum_vector_elements();
    //cout << n_hist << endl;
    
    TEST_TRUE(fabs(sum - 1.0) < FLT_EPSILON);
    RETURN_VICTORIOUSLY();

}
