/* $Id$ */
/* ===========================================================================*
 |
 |  Copyright (c) 1994-2011 by Kobus Barnard (author)
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
 |  Author:  Emily Hartley
 * ===========================================================================*/

#include <g_cpp/g_circle.h>
#include <iostream>
#include <fstream>
#include <m_cpp/m_matrix_stream_io.h>
#include <sstream>

/*
Implementation was derived from the forumulas here:
http://paulbourke.net/geometry/circlefrom3/
*/

#warning "[CODE POLICE] This file needs cleanup to conform to libkjb style."

using namespace kjb;

Circle::Circle() : center(2, 0.0)
{

}

Circle::Circle(Vector* pt1, Vector* pt2, Vector* pt3) : center(2,0.0)
{
    this->radius = -1;      // error checking 

    if (!this->IsPerpendicular(pt1, pt2, pt3) )             this->CalcCircle(pt1, pt2, pt3);    
    else if (!this->IsPerpendicular(pt1, pt3, pt2) )        this->CalcCircle(pt1, pt3, pt2);    
    else if (!this->IsPerpendicular(pt2, pt1, pt3) )        this->CalcCircle(pt2, pt1, pt3);    
    else if (!this->IsPerpendicular(pt2, pt3, pt1) )        this->CalcCircle(pt2, pt3, pt1);    
    else if (!this->IsPerpendicular(pt3, pt2, pt1) )        this->CalcCircle(pt3, pt2, pt1);    
    else if (!this->IsPerpendicular(pt3, pt1, pt2) )        this->CalcCircle(pt3, pt1, pt2);    
    else { 
        return;
    }
}

Circle::Circle(Matrix* points) : center(2, 0.0){
    //TODO fix
    //(*this) =  computeCircleGivenPoints(Matrix* mp);
}

Circle::Circle(const std::vector<kjb::Vector> & ipoints) : center(2,0.0) 
{
    (*this) =  computeCircleGivenPoints(ipoints);
}

Circle::Circle(Vector* c, double radius) : center(2, 0.0){
    this->radius = radius;
    this->center = *c;
}

bool Circle::IsPerpendicular(Vector *pt1, Vector *pt2, Vector *pt3){
    double yDelta_a= pt2->at(1) - pt1->at(1);
    double xDelta_a= pt2->at(0) - pt1->at(0);
    double yDelta_b= pt3->at(1) - pt2->at(1);
    double xDelta_b= pt3->at(0) - pt2->at(0);
    

    if (fabs(yDelta_a) <= 0.0000001){
        return true;
    }
    else if (fabs(yDelta_b) <= 0.0000001){
        return true;
    }
    else if (fabs(xDelta_a)<= 0.000000001){
        return true;
    }
    else if (fabs(xDelta_b)<= 0.000000001){
//      TRACE(" A line of two point are perpendicular to y-axis 2\n");
        return true;
    }
    else return false ;
}

double Circle::CalcCircle(Vector* pt1, Vector* pt2, Vector* pt3)
{
    double yDelta_a= pt2->at(1) - pt1->at(1);
    double xDelta_a= pt2->at(0) - pt1->at(0);
    double yDelta_b= pt3->at(1) - pt2->at(1);
    double xDelta_b= pt3->at(0) - pt2->at(0);
    double aSlope=yDelta_a/xDelta_a; // 
    double bSlope=yDelta_b/xDelta_b;

//  printf("aslope = %f, bslope = %f\n",aSlope,bSlope);

    float x = (aSlope*bSlope*(pt1->at(1) - pt3->at(1)) + bSlope*(pt1->at(0) + pt2 ->at(0))
                - aSlope*(pt2->at(0)+pt3->at(0)) )/(2* (bSlope-aSlope));
    float y = -1*(x - (pt1->at(0)+pt2->at(0))/2)/aSlope + (pt1->at(1)+pt2->at(1))/2;
    // calc center
    //printf("calc circle\n");

    this->center(0) = x;
    this->center(1) = y;

    this->radius = sqrt(pow(center(0)-pt1->at(0),2) + pow(center(1)-pt1->at(1),2));     // calc. radius

    return this->radius;
}

const Vector & Circle::GetCenter()
{
    return center;
}

double Circle::GetRadius()
{
    return radius;
}

Circle Circle::computeCircleGivenPoints(Matrix* mp){

    const int passes = 200;
    int i = 0;
    int j;
    int n_rows = mp -> get_num_rows();
    Circle* best_circle = NULL;
    double best_distance = 0;
        
    
    if(n_rows < 3){
        printf("Need 3 or more points!\n");
        exit(1);
    }   

    Vector* candidates[3];
    candidates[0] = new Vector(2);
    candidates [1] = new Vector(2);
    candidates[2] = new Vector(2);
    int indexes[3];

    for(i = 0; i<passes; i++){

        for(j = 0; j<3; j++){
            indexes[j] = -1;
        }
                for(j= 0; j<3;){
                        int randindex = (((float)rand())/RAND_MAX) * n_rows;

                        if(randindex >= n_rows){randindex = n_rows-1;}  
            if(indexes[0] == randindex || indexes[1] == randindex){continue;}
        
            //printf("nextindex = %d\n",randindex); 
                        candidates[j] = & candidates[j]->set(mp->get_row(randindex)[0], mp->get_row(randindex)[1]);
            indexes[j] = randindex;               
            j++;
        }
        //printf("using points (%f, %f), (%f,%f), (%f,%f)\n",candidates[0]->at(0),candidates[0]->at(1),candidates[1]->at(0),candidates[1]->at(1),candidates[2]->at(0),candidates[2]->at(1));    
        Circle c = Circle(candidates[0],candidates[1],candidates[2]);
        //printf("Got center: %f %f\n",c.GetCenter()->at(0),c.GetCenter()->at(1));
        //printf("Got radius: %f\n",c.GetRadius());

        const Vector & found_center = c.GetCenter();
        double found_radius = c.GetRadius(); 

        //Calculate average distance, num outliers. Set best circle if needed
        double average_distance = 0;
        int num_outliers = 0;
        for(j = 0; j<n_rows; j++){
            if(j != indexes[0] && j != indexes[1] && j != indexes[2]){
                double next_distance = sqrt(pow(mp->get_row(j).at(0)-found_center(0),2) + pow(mp->get_row(j).at(1)-found_center(1),2));                 
                average_distance += fabs(next_distance - found_radius);
                if(fabs(next_distance - found_radius) > found_radius/3){
                    num_outliers++;
                }
            }       
        }
        average_distance /= n_rows;
//      printf("next avg dista = %f, nout = %d\n",average_distance,num_outliers);
        if(num_outliers <= n_rows/10){
            if(best_distance == 0 || average_distance < best_distance){
                best_distance = average_distance;
                best_circle = &c;
            }
        }
        
    }

    return *best_circle;
}

Circle Circle::computeCircleGivenPoints(const std::vector<kjb::Vector> & ipoints){

    const int passes = 200;
    int i = 0;
    int j;
    int n_rows = ipoints.size();
    Circle best_circle;
    double best_distance = 0;
        
    
    if(n_rows < 3){
        printf("Need 3 or more points!\n");
        exit(1);
    }   

    Vector* candidates[3];
    candidates[0] = new Vector(2);
    candidates [1] = new Vector(2);
    candidates[2] = new Vector(2);
    int indexes[3];

    for(i = 0; i<passes; i++){

        for(j = 0; j<3; j++){
            indexes[j] = -1;
        }
                for(j= 0; j<3;){
                        int randindex = (((float)rand())/RAND_MAX) * n_rows;

                        if(randindex >= n_rows){randindex = n_rows-1;}  
            if(indexes[0] == randindex || indexes[1] == randindex){continue;}
        
            //printf("nextindex = %d\n",randindex); 
                        candidates[j] = & candidates[j]->set(ipoints[randindex](0), ipoints[randindex](1));
            indexes[j] = randindex;               
            j++;
        }
        //printf("using points (%f, %f), (%f,%f), (%f,%f)\n",candidates[0]->at(0),candidates[0]->at(1),candidates[1]->at(0),candidates[1]->at(1),candidates[2]->at(0),candidates[2]->at(1));    
        Circle c(candidates[0],candidates[1],candidates[2]);
        //printf("Got center: %f %f\n",c.GetCenter()->at(0),c.GetCenter()->at(1));
        //printf("Got radius: %f\n",c.GetRadius());

        const Vector & found_center = c.GetCenter();
        double found_radius = c.GetRadius(); 

        //Calculate average distance, num outliers. Set best circle if needed
        double average_distance = 0;
        int num_outliers = 0;
        for(j = 0; j<n_rows; j++){
            if(j != indexes[0] && j != indexes[1] && j != indexes[2]){
                double next_distance = sqrt(pow(ipoints[j](0)-found_center(0),2) + pow(ipoints[j](1)-found_center(1),2));                   
                average_distance += fabs(next_distance - found_radius);
                if(fabs(next_distance - found_radius) > found_radius/3){
                    num_outliers++;
                }
            }       
        }
        average_distance /= n_rows;
//      printf("next avg dista = %f, nout = %d\n",average_distance,num_outliers);
        if(num_outliers <= n_rows/10){
            if(best_distance == 0 || average_distance < best_distance){
                best_distance = average_distance;
                best_circle = c;
            }
        }
        
    }

    delete candidates[0];
    delete candidates[1];
    delete candidates[2];
    return best_circle;
}

Image Circle::draw_circle(Matrix* mp){

    Image::Pixel_type p;
    memset(&p,0,sizeof(Image::Pixel_type));
    p.r = 255;
    p.g = 0;
    p.b = 0;

    const Vector & center = this->GetCenter();
    double radius = this->GetRadius();
    double xoffset = 0;
    double yoffset = 0;
        
    if(center(0) - radius < 0){
        xoffset = -(center(0) - radius);
    }

    if(center(1) - radius < 0){
        yoffset = -(center(1) -radius);
    }

    kjb::Image i = Image(GetCenter()(1) + this->GetRadius() + 50+yoffset,this->GetCenter()(0) + this->GetRadius() +50+xoffset);
    
    i.draw_circle((int)this->GetCenter()(1)+yoffset,(int)this->GetCenter()(0)+xoffset,(int)this->GetRadius(),2,p);
    printf("Image dims (%d %d)\n",i.get_num_rows(),i.get_num_cols());
    p.r = 0;
    p.b = 255;
    int j;
    for(j = 0; j<mp->get_num_rows();j++){
        i.draw_point(mp->get_row(j).at(1)+yoffset,mp->get_row(j).at(0)+xoffset,2,p);        
    }

    return i;   

}

/*
int main(int argc, char** argv){

    if(argc == 1){return 0;}


    char* mat = argv[1];    

    Matrix mp;

    mp.read(mat);

    int i,j;

    for(i = 0; i<mp.get_num_rows(); i++){

        for(j = 0; j<mp.get_num_cols(); j++){
            printf("%f ",mp.at(i,j));
        }
        printf("\n");
    }

    //Circle cir = computeCircleGivenPoints(&mp);
    Circle* c = new Circle(&mp);

    printf("Got radius %f\n",c->GetRadius());
        printf("Got center: %f %f\n",c->GetCenter()->at(0),c->GetCenter()->at(1));

    Image get = c->draw_circle(&mp);
    get.write(argv[2]);

    return 0;
}
*/




/**
 * Performs a deep copy of the center point, the radius, and the normal vector.
 *
 * @param  c  Circle_in_3d to copy into this one.
 *
 * @return  A reference to this circle in 3d space.
 */
Circle_in_3d& Circle_in_3d::operator= (const Circle_in_3d& c)
{
    circle_center = c.circle_center;
    circle_radius = c.circle_radius;
    circle_normal = c.circle_normal;

    return *this;
}


void Circle_in_3d::read(std::istream & in)
{
    using std::ostringstream;
    using std::istringstream;

    const char * field_value;

    // Radius
    if(!(field_value = read_field_value(in, "radius")))
    {
        KJB_THROW_2(Illegal_argument, "Missing radius");
    }
    istringstream ist(field_value);
    ist >> circle_radius;
    if(circle_radius < 0)
    {
        KJB_THROW_2(Illegal_argument, "Radius must be bigger than 0");
    }
    if(ist.fail())
    {
        KJB_THROW_2(Illegal_argument, "Missing radius");
    }
    ist.clear(std::ios_base::goodbit);

    // Center
    if(!(field_value = read_field_value(in, "center")))
    {
        KJB_THROW_2(Illegal_argument,"Missing center");
    }
    ist.str(field_value);
    ist >> circle_center(0) >> circle_center(1) >> circle_center(2);
    if(ist.fail())
    {
        KJB_THROW_2(Illegal_argument, "Missing center");
    }
    ist.clear(std::ios_base::goodbit);

    // Normal
    if(!(field_value = read_field_value(in, "normal")))
    {
        KJB_THROW_2(Illegal_argument,"Missing normal");
    }
    ist.str(field_value);
    ist >> circle_normal(0) >> circle_normal(1) >> circle_normal(2);
    if(ist.fail())
    {
        KJB_THROW_2(Illegal_argument, "Missing normal");
    }
    ist.clear(std::ios_base::goodbit);
}

void Circle_in_3d::write(std::ostream & out) const
{
    out << "radius: " << circle_radius << '\n'
        << "center: " << circle_center(0) << ' ' 
                      << circle_center(1) << ' '
                      << circle_center(2) << '\n'
        << "normal: " << circle_normal(0) << ' '
                      << circle_normal(1) << ' '
                      << circle_normal(2) << '\n';
}
