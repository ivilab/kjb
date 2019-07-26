/* $Id: g_hull.cpp 21755 2017-09-07 21:54:51Z kobus $ */
/* =========================================================================== *
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
   |  Author: Jinyan Guan 
 * =========================================================================== */

#include <g_cpp/g_hull.h>
#include <h/h_qh.h>
#include <h/h_intersect.h>
#include <iostream>

namespace kjb
{
/** /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ */
int get_convex_hull
(
    const kjb::Matrix& points,
    Matrix& hull_vertices,
    std::vector<Matrix>& hull_facets
)
{
    using namespace kjb_c;
    Hull* hp = NULL;
    int result = get_convex_hull(&hp, points.get_c_matrix(), DEFAULT_HULL_OPTIONS);
    if(result == ERROR)
    {
        free_hull(hp);
        return ERROR;
    }

    hull_vertices = kjb::Matrix(*(hp->vertex_mp));
    hull_facets.clear();
    hull_facets.resize(hp->facets->length);
    for(int i = 0; i < hp->facets->length; i++)
    {
        hull_facets[i] = kjb::Matrix(*(hp->facets->elements[i]));
    }

    free_hull(hp);
    return result;
}

/** /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ */
bool intersect_hulls
(   
    const std::vector<Matrix>& pts,
    Matrix& hull_vertices,
    std::vector<Matrix>& hull_facets
)
{
    using namespace kjb_c;
    Hull* result_hp = NULL;
    Queue_element* hull_list_head = NULL;
    Queue_element* hull_list_end = NULL;

    std::vector<Matrix>::const_iterator pts_iter;
    for(pts_iter = pts.begin(); pts_iter != pts.end(); pts_iter++)
    {
        Hull* hp = NULL;
        EPETE(get_convex_hull(&hp, pts_iter->get_c_matrix(), DEFAULT_HULL_OPTIONS));
        EPETE(insert_into_queue(&hull_list_head, &hull_list_end, hp));
    }
    
    EPETE(set_hull_options("hir", "100")); 
    EPETE(set_qhull_options("qhull-error-file", "qhull-error-log")); 
    EPETE(set_hull_options("hull-intersection-method", "dual")); 

    int intersect;
    EPE(intersect = intersect_hulls(hull_list_head, DEFAULT_HULL_OPTIONS, &result_hp));
    if (intersect == ERROR)
    {
        kjb_print_error();
        free_queue(&hull_list_head, (Queue_element**)NULL, 
                  (void(*)(void*))free_hull); 
        return false;
    }
    else if (result_hp == NULL)
    {
        std::cout<<"The resulting intersection of the convex hulls is NULL!\n";
        free_queue(&hull_list_head, (Queue_element**)NULL, 
                  (void(*)(void*))free_hull); 
        return false;
    }
    else if (intersect == NO_SOLUTION)
    {
        std::cout<<"No solution found for intersecting the convex hulls!\n";
        free_queue(&hull_list_head, (Queue_element**)NULL, 
                  (void(*)(void*))free_hull); 
        return false;
    }

    // create a matrix that contains the points and facets in the resulting hull
    hull_vertices = kjb::Matrix(*(result_hp->vertex_mp));
    hull_facets.clear();
    hull_facets.resize(result_hp->facets->length);
    for(int i = 0; i < result_hp->facets->length; i++)
    {
        hull_facets.push_back(kjb::Matrix(*(result_hp->facets->elements[i])));
    }

    // clean  up
    free_queue(&hull_list_head, &hull_list_end, 
              (void(*)(void*))free_hull); 
    free_hull(result_hp);
    return true;
}
} //namespace kjb
