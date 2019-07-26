/* $Id: test_weight_array.cpp 17426 2014-08-30 00:36:27Z predoehl $ */

/*!
 * @file test_weight_array.cpp
 *
 * @author Colin Dawson 
 */

#include <prob_cpp/prob_weight_array.h>
#include <vector>
#include <iostream>

int main()
{
    using namespace kjb;
    
    typedef boost::array<double, 3> Arr3;
    typedef boost::array<double, 5> Arr5;
    typedef kjb::Weight_array<5> WA5;
    typedef kjb::Weight_array<3> WA3;
    typedef WA5::Filter Filter;
    typedef boost::array<WA5, 3> WAA;
    size_t errors = 0;
    size_t num_tests = 0;

    Arr5 aa = {{1.0, 1.0, 2.0, 3.0, 3.0}};
    Arr5 atwo = {{2.0, 2.0, 2.0, 2.0, 2.0}};
    Arr5 aone = {{1.0, 1.0, 1.0, 1.0, 1.0}};
    Arr5 anormed = {{0.1, 0.1, 0.2, 0.3, 0.3}};
    Arr5 azero = {{0.0, 0.0, 0.0, 0.0, 0.0}};
    Arr5 amean = {{0.9, 0.9, 1.0, 1.1, 1.1}};
    Arr5 afiltered = {{1.0, 0.0, 2.0, 0.0, 3.0}};
    
    Arr3 lc = {{0.1, 0.2, 0.3}};

    Filter filter = {{1, 0, 1, 0, 1}};

    WA5 a(aa);
    WA5 two(atwo);
    WA5 one(aone);
    WA5 zero(azero);
    WA5 normed(anormed);
    WA5 mean(amean);
    WA5 filtered(afiltered);
    WA3 wts(lc);

    WAA components = {{a, one, two}};

    ++num_tests;
    std::cerr << num_tests << ": Vector of ones is " << one << std::endl;
    if(one == zero)
    {
    std::cerr << "Error: weights not getting initialized." << std::endl;
    ++errors;
    }
    ++num_tests;
    std::vector<WA5::Val_type> v(a.cbegin(), a.cend());
    WA5 b(v.begin(), v.end());
    std::cerr << num_tests << ": Copy of " << a << " is " << b << std::endl;
    if(a != b)
    {
    std::cerr << "Error: iterator constructor is malfunctioning."
          << std::endl;
    ++errors;
    }
    ++num_tests;
    std::cerr << num_tests << ": "
          << a << " + " << zero << " == " << a + zero << std::endl;
    if(a + zero != a)
    {
    std::cerr << "Error: element-wise addition of zero alters weights."
          << std::endl;
    ++errors;
    }
    ++num_tests;
    std::cerr << num_tests << ": "
          << a << " * " << zero << " == " << a * zero << std::endl;
    if(a * zero != zero)
    {
    std::cerr << "Error: element-wise multiplication by zero is not zero."
          << std::endl;
    ++errors;
    }
    ++num_tests;
    std::cerr << num_tests << ": "
          << a << " + 0 == " << a + 0 << std::endl;
    if(a + 0 != a)
    {
    std::cerr << "Error: addition of zero alters weights."
          << std::endl;
    ++errors;
    }
    ++num_tests;
    std::cerr << num_tests << ": "
          << a << " * 0 == " << a * 0 << std::endl;
    if(a * 0 != zero)
    {
    std::cerr << "Error: multiplication by zero is not zero."
          << std::endl;
    ++errors;
    }
    ++num_tests;
    std::cerr << num_tests << ": "
          << a << " * 1 == " << a * 1 << std::endl;
    if(a * 1 != a)
    {
    std::cerr << "Error: multiplication by unity changes weights."
          << std::endl;
    ++errors;
    }
    ++num_tests;
    std::cerr << num_tests << ": "
          << one << " + " << one << " == " << one + one << std::endl;
    if(one + one != two)
    {
    std::cerr << "Error: weight addition not working properly." << std::endl;
    ++errors;
    }
    ++num_tests;
    std::cerr << num_tests << ": "
          << one << " + 1 == " << one + 1 << std::endl;
    if(one + 1 != two)
    {
    std::cerr << "Error: const addition not working properly." << std::endl;
    
    ++errors;
    }
    ++num_tests;
    std::cerr << num_tests << ": "
          << one << " * " << two << " == " << one*two << std::endl;
    if(one * two != two)
    {
    std::cerr << "Error: const addition not working properly." << std::endl;
    ++errors;
    }
    ++num_tests;
    std::cerr << num_tests << ": " << a << " * " << filter
          << " == " << (a *= filter) << std::endl;
    if(a != filtered)
    {
    std::cerr << "Error: Filtering failed." << std::endl;
    std::cerr << "Result should be " << filtered << std::endl;
    }
    ++num_tests;
    a.normalize();
    std::cerr << num_tests << ": Normalized weights are " << a << std::endl;
    if(a != normed)
    {
        std::cerr << "Error: normalization failed." << std::endl;
        std::cerr << "Normalized weights are " << a << std::endl;
        std::cerr << "Normed is " << normed << std::endl;
    std::cerr << "Ratio is " << a / normed << std::endl;
        ++errors;
    }
    ++num_tests;
    a += 1; a.normalize(); normed.normalize();
    std::cerr << num_tests << ": Normalized weights are " << a << std::endl;
    if(a != normed)
    {
    std::cerr << "Error: normalization after addition failed." << std::endl;
    std::cerr << "Normed is " << normed << std::endl;
    std::cerr << "Ratio is " << a / normed << std::endl;
    ++errors;
    }
    ++num_tests;
    a *= 2; a.normalize();
    std::cerr << num_tests << ": Normalized weights are " << a << std::endl;
    if(a != normed)
    {
    std::cerr << "Error: normalization after multiplication failed."
          << std::endl;
    std::cerr << "Normed is " << normed << std::endl;
    std::cerr << "Ratio is " << a / normed << std::endl;
    ++errors;
    }
    ++num_tests;
    a /= 2; a.normalize();
    std::cerr << num_tests << ": Normalized weights are " << a << std::endl;
    if(a != normed)
    {
    std::cerr << "Error: normalization after division failed."
          << std::endl;
    std::cerr << "Normed is " << normed << std::endl;
    std::cerr << "Ratio is " << a / normed << std::endl;
    ++errors;
    }
    ++num_tests;
    WA5 convex = convex_combination(wts, components);
    std::cerr << num_tests << ": Average weights are " << convex << std::endl;
    if(convex != mean)
    {
    std::cerr << "Error: convex combination failed."
          << std::endl;
    std::cerr << "Should be " << mean << std::endl;
    ++errors;
    }
    std::cout << num_tests - errors << " out of "
          << num_tests << " tests passed." << std::endl;
    if(errors == 0) return 0;
    else return 1;
};

