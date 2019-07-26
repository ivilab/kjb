/* =========================================================================== *
|
|  Copyright (c) 1994-2008 by Kobus Barnard (author).
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
| Authors:
|     Jinyan Guan
|
* =========================================================================== */

/* $Id$ */

#include <video_cpp/video_background.h>
#include <l_cpp/l_filesystem.h>
#include <i_cpp/i_image.h>
#include <string>
#include <vector>

using namespace kjb;
using namespace std;
int main(int argc, char** argv)
{
    if(argc != 3)
    {
        cout << " Usage: ./background movie-dir output-dir\n";
        return EXIT_SUCCESS;
    }
    string movie_dir = argv[1];
    string out_dir = argv[2];

    vector<string> frames_fps = 
        file_names_from_format(movie_dir + "/frames/%05d.jpg");

    Matrix r_mat;
    Matrix g_mat;
    Matrix s_mat;
    compute_median_background(frames_fps, r_mat, g_mat, s_mat);
   
    //r_mat /= (max(r_mat)/255.0);
    //g_mat /= (max(g_mat)/255.0);
    //s_mat /= (max(s_mat)/255.0);
    size_t num_rows = r_mat.get_num_rows();
    size_t num_cols = r_mat.get_num_cols();
    Image bg(num_rows, num_cols);
    for(size_t i = 0; i < num_rows; i++)
    {
        for(size_t j = 0; j < num_cols; j++)
        {
            bg(i, j, Image::RED) = r_mat(i, j) * s_mat(i, j);
            bg(i, j, Image::GREEN) = g_mat(i, j) * s_mat(i, j);
            bg(i, j, Image::BLUE) = s_mat(i, j) - bg(i, j, Image::RED) 
                                    - bg(i, j, Image::GREEN);
        }
    }

    bg.write(string(out_dir + "/bg.jpg").c_str());

    return EXIT_SUCCESS;

}

