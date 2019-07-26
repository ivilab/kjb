#include <m_cpp/m_vector.h>
#include <vector>
#include <g2_cpp/g2_ransac_line_fitting.h>
#include "test.h"

using namespace kjb;
int main(int argc, char** argv)
{
    std::vector<kjb::Vector> points(11);
    points[0] = kjb::Vector(1.0, 1.0); //
    points[1] = kjb::Vector(2.0, 2.0); //
    points[2] = kjb::Vector(3.0, 3.0); //
    points[3] = kjb::Vector(4.0, 10.0);
    points[4] = kjb::Vector(6.0, 6.0); //
    points[5] = kjb::Vector(7.0, 7.0); //
    points[6] = kjb::Vector(1.0, 10.0);
    points[7] = kjb::Vector(8.0, 8.0); //
    points[8] = kjb::Vector(14.0, 14.0); //
    points[9] = kjb::Vector(20.0, 20.0); //
    points[10] = kjb::Vector(20.0, 2.0);
    
    Ransac_line_fitting ransac(points, 6, 1.0);
    ransac.run(100);
    kjb::Line fitted_line = ransac.get_best_line();
    double error = ransac.get_best_fitting_error();

    //std::cout<<"error: "<<error<<std::endl;
    //std::cout<<"Line: "<<fitted_line.get_a()<<" "<<fitted_line.get_b()<<" "<<fitted_line.get_c()<<std::endl;
    TEST_TRUE(fabs(fitted_line.get_a()-1.0) < FLT_EPSILON);
    TEST_TRUE(fabs(fitted_line.get_c()) < FLT_EPSILON);
    return EXIT_SUCCESS;
}
