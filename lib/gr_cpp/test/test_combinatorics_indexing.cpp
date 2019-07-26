/* $Id$ */
/*=========================================================================== *
  |
  | Copyright (c) 1994-2010 by Kobus Barnard (author)
  |
  | Personal and educational use of this code is granted, provided that this
  | header is kept intact, and that the authorship is not misrepresented, that
  | its use is acknowledged in publications, and relevant papers are cited.
  |
  | For other use contact the author (kobus AT cs DOT arizona DOT edu).
  |
  | Please note that the code in this file has not necessarily been adequately
  | tested. Naturally, there is no guarantee of performance, support, or fitness
  | for any particular task. Nonetheless, I am interested in hearing about
  | problems that you encounter.
  |
  | Author:  Emily Hartley
 *=========================================================================== */

// This code is used in find_all_circles_in_polymesh() (located in 
// gr_find_shapes.cpp).

#include <iostream>

#include <gr_cpp/gr_find_shapes.h>

#include <vector>

using namespace kjb;


int main()
{
    int num_vertices = 7;
    std::vector<int> pointList;
    pointList.push_back(0);
    pointList.push_back(1);
    pointList.push_back(2);
    pointList.push_back(3);
    pointList.push_back(4);
    pointList.push_back(5);
    pointList.push_back(6);


    std::vector<int> circleFound;
    // Create a vector of size num_vertices^3 with all values initialized 
    // to 0.
    for(int i = 0; i < num_vertices-2; i++)
    {
        for(int j = i+1; j < num_vertices-1; j++)
        {
            for(int k = j+1; k < num_vertices; k++)
            {
                circleFound.push_back(-1);
            }
        }
    }

    // Get every combination of 3 points.
    int pointList_size = pointList.size();
    for(int p1 = 0; p1 < pointList_size - 2; p1++)
    {
        long long index1 = 0;
        if(p1 > 0)
        {
            // summation from k = (num-p1) to (num-1) of k choose 2
            //     equals (num choose 3) - ((num - p1) choose 3)
            index1 = ((num_vertices*(num_vertices-1)*(num_vertices-2)) - 
                      ((num_vertices-p1)*(num_vertices-p1-1)
                                        *(num_vertices-p1-2)))/6;
        }
        for(int p2 = p1+1; p2 < pointList_size - 1; p2++)
        {
            long long index2 = 0;
            if(p2 > p1 + 1)
            {
                double tmp2 = ((p2 - (p1 + 1)) * num_vertices) - 
                              (((p2/2.0)*(1+p2)) - (((p1+1)/2.0)*(1+p1+1)));    
                index2 = tmp2;
            }
            for(int p3 = p2+1; p3 < pointList_size; p3++)
            {
                // Check if the combination has been checked yet by looking 
                // in circleFound.
                    
                long long index3 = p3 - (p2 + 1);
                long long pos = index1 + index2 + index3;
                if(circleFound[pos] == -1)   
                {
                    // Fit a circle to the points since it has not been 
                    // done yet.

                    // Change the value at pos in circleFound to 1.
                    circleFound[pos] = pos;
                }
            }
        }
    }
    
    for(int i = 0; i < circleFound.size(); i++)
    {
        std::cout << "position " << i << ": " << circleFound[i] << std::endl;
    }
}
