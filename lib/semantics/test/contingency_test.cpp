/*!
 * @file Contingency_test.cpp
 *
 * @author Colin Dawson 
 * $Id: contingency_test.cpp 17426 2014-08-30 00:36:27Z predoehl $
 */

#include "l/l_init.h"
#include "l/l_incl.h"
#include "semantics/Marginal_cell.h"
#include <iostream>

int main(int, char**)
{
    kjb_c::kjb_init();
    // if(! kjb_c::is_interactive())
    // {
    //  return EXIT_SUCCESS;
    // }
    
    // const int a[4] = {0,0,0,0};
    // const int b[4] = {0,0,0,1};
    // const int c[4] = {0,0,1,0};
    // const int d[4] = {0,0,1,1};
    // const int e[4] = {1,0,1,1};
    // const int f[4] = {1,1,1,1};

    // std::vector<int> A(a, a + 4);
    // std::vector<int> B(b, b + 4);
    // std::vector<int> C(c, c + 4);
    // std::vector<int> D(d, d + 4);
    // std::vector<int> E(e, e + 4);
    // std::vector<int> F(f, f + 4);

    // Categorical_event<1> ea(A, 4);
    // Categorical_event<1> eb(B, 4);
    // Categorical_event<1> ec(C, 4);
    // Categorical_event<1> ed(D, 4);
    // Categorical_event<1> ee(E, 4);
    // Categorical_event<1> ef(F, 4);

    // add_event<int>(ea);
    // add_event<int>(eb);
    // add_event<int>(ec);
    // add_event<int>(ed);
    // add_event<int>(ee);
    // add_event<int>(ef);

    // // std::cout << Marginal_cell<int, 0>::map << std::endl;
    // std::cout << Marginal_cell<int, 1>::map << std::endl;
    // std::cout << Marginal_cell<int, 2>::map << std::endl;
    // std::cout << Marginal_cell<int, 3>::map << std::endl;
    // std::cout << Marginal_cell<int, 4>::map << std::endl;

    return EXIT_SUCCESS;
}

    
    
