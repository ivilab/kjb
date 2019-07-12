#include "m_cpp/m_matrix.h"
#include <iostream>

using namespace kjb;
using namespace std;

double add1(double x) {
    return x + 1;
}

double square(double x) {
    return x * x;
}

double add(double x, double y) {
    return x + y;
}

double multiply(double x, double y) {
    return x * y;
}

bool is_even(double x) {
    return ((int)x) % 2 == 0;
}

int main() {

    Matrix a("matrix1.txt");
    
    cout << a << endl;
    cout << a.map(&add1) << endl;
    cout << a.map(&square) << endl;
    
    cout << a.reduce(&add) << endl;
    cout << a.reduce(&multiply, 1) << endl;
    cout << endl;
    
    cout << a.filter(&is_even) << endl;
    
    return 0;
}