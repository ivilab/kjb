#ifndef GR_SPHERE
#define GR_SPHERE
#include <m_cpp/m_vector.h>
#include <m_cpp/m_matrix.h>
#include "m/m_incl.h"
#include <i_cpp/i_image.h>
#include <i/i_draw.h>

namespace kjb
{

class Sphere{
public:
    double GetRadius() const;
    const kjb::Vector & GetCenter() const;
    //Contructor for radius & center
    Sphere(kjb::Vector & center, double radius);
    Sphere();
    void drawSphere(void);
    static void init();
private:
    double radius;
    kjb::Vector center;
};

}
#endif
