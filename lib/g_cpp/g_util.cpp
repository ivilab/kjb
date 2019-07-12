/* $Id: g_util.h 10603 2011-09-29 19:50:27Z predoehl $ */
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
   |  Author:  Kyle Simek
 * =========================================================================== */


#include "g_cpp/g_util.h"
#include <m_cpp/m_vector.h>
#include <m_cpp/m_matrix.h>
#include <l_cpp/l_exception.h>

namespace kjb { 
namespace geometry { 

Matrix get_translation_matrix(const Vector& v)
{
    size_t d = v.get_length();
    IFT(d == 2 ||  d == 3, Illegal_argument,
        "Translation matrices must be in 2D or 3D.");

    Matrix M = create_identity_matrix(d + 1);
    M.set_col(d, Vector(v).resize(d + 1, 1.0));

    return M;
}

Matrix get_rotation_matrix(double theta)
{
    Matrix M(2, 2);
    M(0, 0) = cos(theta); M(0, 1) = -sin(theta);
    M(1, 0) = sin(theta); M(1, 1) = cos(theta);

    return M;
}

Matrix get_rotation_matrix(const Vector& u, const Vector& v)
{
    const int d = u.get_length();

    IFT(v.get_length() == d, Illegal_argument,
        "Can't compute rotation matrix: vectors must be same dimensionality.");

    IFT(fabs(u.magnitude() - 1.0) < FLT_EPSILON
            && fabs(v.magnitude() - 1.0) < FLT_EPSILON,
        Illegal_argument,
        "Can't compute rotation matrix: vectors must be normalized.");

    if(vector_distance(u, v) < FLT_EPSILON)
    {
        return create_identity_matrix(d);
    }

    size_t rd;
    bool bad_dim = true;
    Vector n;
    Vector vp;
    for(int i = 0; i < d && bad_dim; ++i)
    {
        vp = v; vp[i] = -vp[i];
        n = vp - u;
        if(n.magnitude() >= FLT_EPSILON)
        {
            bad_dim = false;
            rd = i;
        }
    }

    assert(!bad_dim);

    Matrix R(d, d);
    for(int i = 0; i < d; ++i)
    {
        Vector x((int)d, 0.0); x[i] = 1.0;
        R.set_col(i, x - 2*(dot(x, n)/dot(n, n))*n);
    }

    R.set_row(rd, -R.get_row(rd));

    return R;
}

Vector projective_to_euclidean(const Vector& v)
{
    if(v.size() == 3)
    {
        return v;
    }

    IFT(v.size() == 4, Illegal_argument, "Vector must be of size 3 or 4.");
    IFT(fabs(v[3] - 0) > 1e-10, Illegal_argument, "Homogeneous coordinate cannot be 0.");

    return Vector(v / v(3)).resize(3);
}

Vector projective_to_euclidean_2d(const Vector& v)
{
    if(v.size() == 2)
    {
        return v;
    }

    IFT(v.size() == 3, Illegal_argument, "Vector must be of size 3 or 4.");
    IFT(fabs(v[2] - 0) > 1e-10, Illegal_argument, "Homogeneous coordinate cannot be 0.");

    return Vector(v / v(2)).resize(2);
}

Vector euclidean_to_projective(const Vector& v)
{
    if(v.size() == 4)
    {
        IFT(fabs(v[3] - 0) > 1e-10, Illegal_argument, "Homogeneous coordinate cannot be 0.");
        return v;
    }

    IFT(v.size() == 3, Illegal_argument, "Vector must be of size 2 or 3.");

    return Vector(v).resize(4, 1.0);
}

Vector euclidean_to_projective_2d(const Vector& v)
{
    if(v.size() == 3)
    {
        IFT(fabs(v[2] - 0) > 1e-10, Illegal_argument, "Homogeneous coordinate cannot be 0.");
        return v;
    }

    IFT(v.size() == 2, Illegal_argument, "Vector must be of size 2 or 3.");

    return Vector(v).resize(3, 1.0);
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

bool is_point_in_polygon_new(Matrix poly,Vector point){

    int intersections = 0;
    double movementfactor = 0.000001;
    movementfactor = movementfactor*(rand()%9);

    //jostle the point around a little to avoid intersecting verticies
    int upordown = rand()%2;
    double tpy;
    if(upordown==0){ tpy = point(1)-movementfactor; }
    else{ tpy = point(1)+movementfactor; }
    
    int xmin=INT_MAX;
    int xmax=INT_MIN;
    int ymin=INT_MAX;
    int ymax=INT_MIN;

    //check the extreme ends of the polygon
    for(int i=0;i<poly.get_num_rows();i++){
        if(poly(i,0)<xmin){ xmin = poly(i,0); }
        if(poly(i,1)<ymin){ ymin = poly(i,1); } 
        if(poly(i,0)>xmax){ xmax = poly(i,0); }
        if(poly(i,1)>ymax){ ymax = poly(i,1); } 
    }

    if(point(0)<xmin || point(0)>xmax ||
       point(1)<ymin || point(1)>ymax){
        //std::cout << " out of bounds ";
        return false; //Point is outside the bounds of the polygon
    }

    /*
    for each line, check to see if the point is to the right 
    of the rightmost point, and then if it is between the 2
     y points. And then if it is to the left of the line.
    */  
    for(int i=0;i<poly.get_num_rows();i++){

        double x1;
        double y1;
        double x2;
        double y2;
            
        //gotta orient the lines from bottom to top, y1 is always the lower 
        if(poly(i,1)<poly((i+1)%poly.get_num_rows(),1)){
            x1 = poly(i,0);
            y1 = poly(i,1);//point 1
            x2 = poly((i+1)%poly.get_num_rows(),0);
            y2 = poly((i+1)%poly.get_num_rows(),1);//point 2 (with wraparound)
        }
        else{
            x1 = poly((i+1)%poly.get_num_rows(),0);
            y1 = poly((i+1)%poly.get_num_rows(),1);//point 2 (with wraparound)
            x2 = poly(i,0);
            y2 = poly(i,1);//point 1
        }
        //std::cout << "checking " <<x1<<","<<y1<<" to "<<x2<<","<<y2<<": ";
        double det = (x2-x1)*(point(1)-y1)-(point(0)-x1)*(y2-y1);
        if(det==0){//on a line
            //actually in the bounds of a line?
        //  std::cout << "inline ";
            if(point(1)>=y1 && point(1)<=y2){
                if((point(0)>=x1 && point(0)<=x2) ||
                   (point(0)>=x2 && point(0)<=x1)){
        //          std::cout << "on line\n";
                    return true;
                }
            }
        //  std::cout << "miss\n";
        }
        else{
            
            if(tpy>y1 && tpy<y2){
                double det = (x2-x1)*(tpy-y1)-(point(0)-x1)*(y2-y1);
                if(det > 0){
        //          std::cout << "hit!\n";
                    intersections++;
                }
        //      else{ std::cout << "miss!\n"; } 
            }
        //  else{ std::cout << "miss!\n"; } 
        }
    
                    

    }

//  std::cout << " intersections: " << intersections << ": ";
    //if even intersetions - outside, if odd, inside
    return (intersections%2)!=0; 

}//end is_point_in_polygon

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

Matrix polygon_to_mask(Matrix poly,int height, int width){

    Matrix mask = create_zero_matrix(height,width);
    
    int xmin=0;
    int xmax=0;
    int ymin=0;
    int ymax=0;

    for(int i=0;i<poly.get_num_rows();i++){
        if(poly(i,0)<xmin){ xmin = poly(i,0); }
        if(poly(i,1)<ymin){ ymin = poly(i,1); } 
        if(poly(i,0)>xmax){ xmax = poly(i,0); }
        if(poly(i,1)>ymax){ ymax = poly(i,1); } 
    }

    kjb::Vector point(0.0,0.0);

    //int xdif = xmax-xmin;
    //int ydif = ymax-ymin;
    for(int h=0;h<=ymax;h++){
        for(int w=0;w<=xmax;w++){
            point(0) = w;
            point(1) = h; 
            mask(h,w) = is_point_in_polygon_new(poly,point);
        }
    }
    return mask;

}//end polygon_to_mask

}}//namespace geometry, namespace kjb

