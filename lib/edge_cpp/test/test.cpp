/* $Id$ */
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

#include <string>
#include <fstream>
#include <iostream>

#include <edge_cpp/edge.h>
#include <i_cpp/i_image.h>

#include "m/m_convolve.h"
#include "m/m_mat_basic.h"
#include "m2/m2_ncc.h"

using namespace std;


/**
 * @brief Gradient element detected using the partial derivative masks.
 *
 * @em dcol and @em drow are normalized. Their original magnitude is preserved
 * in @em mag.
 */
typedef struct
{
    /** @brief Rate of change in brightness along the columns at the point. */
    float dcol;

    /** @brief Rate of change in brightness along the rows at the point. */
    float drow;

    /** @brief Magnitude of the gradient. */
    float mag;

    /** @brief Boolean used for the edge detection algorithm. */
    uint8_t marked;
}
Gradient;


int main()
{
    using namespace kjb;

    Image img("001.jpg");

    Canny_edge_detector edge_detector(1.0, 0.01*255, 0.008*255, 10, true);
    Edge_set_ptr ptr = edge_detector(img);
    if(!ptr->is_edge_set_consistenct())
    {
        std::cout << "EDGESET not consistent!" << std::endl;
    }
    else
    {
        std::cout << "EDGESET OK" << std::endl;
    }
    ptr->remove_short_edges(45);
    if(!ptr->is_edge_set_consistenct())
    {
        std::cout << "EDGESET not consistent!" << std::endl;
    }
    else
    {
        std::cout << "EDGESET OK" << std::endl;
    }
    ptr->break_edges_at_corners(0.89, 6);
    if(!ptr->is_edge_set_consistenct())
    {
        std::cout << "EDGESET not consistent!" << std::endl;
    }
    else
    {
        std::cout << "EDGESET OK" << std::endl;
    }
    ptr->break_edges_at_corners(0.9, 20);
    if(!ptr->is_edge_set_consistenct())
    {
        std::cout << "EDGESET not consistent!" << std::endl;
    }
    else
    {
        std::cout << "EDGESET OK" << std::endl;
    }
    ptr->remove_short_edges(15);
    if(!ptr->is_edge_set_consistenct())
    {
        std::cout << "EDGESET not consistent!" << std::endl;
    }
    else
    {
        std::cout << "EDGESET OK" << std::endl;
    }
    ptr->randomly_color(img);
    if(!ptr->is_edge_set_consistenct())
    {
        std::cout << "EDGESET not consistent!" << std::endl;
    }
    else
    {
        std::cout << "EDGESET OK" << std::endl;
    }
    img.write("001edges.jpg");


    Edge_set_ptr ptr2(new kjb::Edge_set(*ptr));
    if(!ptr2->is_edge_set_consistenct())
    {
        std::cout << "EDGESET not consistent!" << std::endl;
    }
    else
    {
        std::cout << "EDGESET OK" << std::endl;
    }

    return 0;
}

