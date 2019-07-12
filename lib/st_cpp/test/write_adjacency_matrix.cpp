/* $Id$ */
/* ===========================================================================*
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
 |  Author:  Emily Hartley
 * ===========================================================================*/


#include <gr_cpp/gr_triangular_mesh.h>
#include <iostream>
#include <fstream>

using namespace kjb;

Triangular_mesh * tm = NULL;
char* fileName = NULL;
char mesh[80];
char* path = "/net/v04/data/delpero/parts_project/dataset/kjb/";
char fullPath[100];

int main(int argc, char* argv[])
{
    try{
        fileName = argv[1];
    }
    catch(KJB_error e)
    {
        e.print(std::cout);
    }

    std::ifstream meshList(fileName);
    if(meshList.is_open())
    {
        for(int i = 0; i < 98; i++)
        {
//            mesh = "";
            meshList.getline(mesh,80);
            strcpy(fullPath, path);
            strcat(fullPath, mesh);
            try{
                tm = new Triangular_mesh(fullPath);
            }
            catch(KJB_error e)
            {
                e.print(std::cout);
            }
            (*tm).set_adjacency_matrix(fullPath);
            free(tm);
        }
        meshList.close();
    }
}
