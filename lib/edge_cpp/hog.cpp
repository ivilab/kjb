
/* $Id $ */

/**
 * @file
 *
 * @author Luca Del Pero
 *
 * @brief Hog class
 *
 * Copyright (c) 2003-2008 by members of University of Arizona Computer Vision
 * group. For use outside the University of Arizona Computer Vision group
 * please contact Kobus Barnard.
 */

#include <edge_cpp/hog.h>
#include <m_cpp/m_flip.h>
#include <sstream>
#include <iostream>

using namespace kjb;

Hog_responses::Hog_responses(const Image & img, int ibin_size) : Readable(), Writeable()
{
    bin_size = ibin_size;
    num_rows = img.get_num_rows();
    num_cols = img.get_num_cols();

    // unit vectors used to compute gradient orientation
    double uu[9] = {1.0000,
            0.9397,
            0.7660,
            0.500,
            0.1736,
            -0.1736,
            -0.5000,
            -0.7660,
            -0.9397};
    double vv[9] = {0.0000,
            0.3420,
            0.6428,
            0.8660,
            0.9848,
            0.9848,
            0.8660,
            0.6428,
            0.3420};

    hog_num_rows = get_hog_num_rows();
    hog_num_cols = get_hog_num_cols();
    int blocks[2];
    blocks[0] = (int)round((double)num_rows/(double)bin_size);
    blocks[1] = (int)round((double)num_cols/(double)bin_size);

    float * hist = new float[blocks[0]*blocks[1]*18];
    float * norm = new float[blocks[0]*blocks[1]];
    std::fill_n(hist, blocks[0]*blocks[1]*18, 0.0);
    std::fill_n(norm, blocks[0]*blocks[1], 0.0);

    hog_size = (blocks[0]-2)*(blocks[1]-2)*32;
    hog_responses = new float[hog_size];
    std::fill_n(hog_responses, hog_size, 0.0);
    int visible[2];
    visible[0] = blocks[0]*bin_size;
    visible[1] = blocks[1]*bin_size;
    std::cout << "img size:" << num_rows << "   " << num_cols << std::endl;
    std::cout << "size:" << blocks[0]-2 << "   " << blocks[1]-2 << std::endl;

    int out[3];
    out[0] = hog_max(blocks[0]-2, 0);
    out[1] = hog_max(blocks[1]-2, 0);
    out[2] = 32;

    for (int x = 1; x < visible[1]-1; x++)
    {
        for (int y = 1; y < visible[0]-1; y++)
        {
            int current_x = hog_min(x, num_cols -2);
            int current_y = hog_min(y, num_rows -2);
             // first color channel
            double dx = img(current_y, current_x + 1, 0) - img(current_y, current_x - 1, 0);
            double dy = img(current_y + 1, current_x, 0) - img(current_y - 1, current_x, 0);
            double v = dx*dx + dy*dy;

            // second color channel
            double dx2 = img(current_y, current_x + 1, 1) - img(current_y, current_x - 1, 1);
            double dy2 = img(current_y + 1, current_x, 1) - img(current_y - 1, current_x, 1);
            double v2 = dx2*dx2 + dy2*dy2;

            // third color channel
            double dx3 = img(current_y, current_x + 1, 2) - img(current_y, current_x - 1, 2);
            double dy3 = img(current_y + 1, current_x, 2) - img(current_y - 1, current_x, 2);
            double v3 = dx3*dx3 + dy3*dy3;

            //pick channel with strongest gradient
            if (v2 > v)
            {
                v = v2;
                dx = dx2;
                dy = dy2;
            }
            if (v3 > v)
            {
                v = v3;
                dx = dx3;
                dy = dy3;
            }

            // snap to one of 18 orientations
            double best_dot = 0;
            int best_o = 0;
            for (int o = 0; o < 9; o++)
            {
                double dot = uu[o]*dx + vv[o]*dy;
                if (dot > best_dot)
                {
                    best_dot = dot;
                    best_o = o;
                }
                else if (-dot > best_dot)
                {
                    best_dot = -dot;
                    best_o = o+9;
                }
            }

            // add to 4 histograms around pixel using linear interpolation
            double xp = ((double)x+0.5)/(double)bin_size - 0.5;
            double yp = ((double)y+0.5)/(double)bin_size - 0.5;
            int ixp = (int)std::floor(xp);
            int iyp = (int)std::floor(yp);
            double vx0 = xp-ixp;
            double vy0 = yp-iyp;
            double vx1 = 1.0-vx0;
            double vy1 = 1.0-vy0;
            v = sqrt(v);

            /*std::cout << "x is: " << x << "   xp is: " <<  xp << "   ixp is: " << ixp;
            std::cout << "  Adding to bin " << ixp << " with weight " << vx1 << "  and to bin " << (ixp + 1) << " with weight " << vx0 << std::endl;*/

            if (ixp >= 0 && iyp >= 0)
            {
                *(hist + ixp*blocks[0] + iyp + best_o*blocks[0]*blocks[1]) += vx1*vy1*v;
            }

            if (ixp+1 < blocks[1] && iyp >= 0)
            {
                *(hist + (ixp+1)*blocks[0] + iyp + best_o*blocks[0]*blocks[1]) += vx0*vy1*v;
            }

            if (ixp >= 0 && iyp+1 < blocks[0])
            {
                *(hist + ixp*blocks[0] + (iyp+1) + best_o*blocks[0]*blocks[1]) += vx1*vy0*v;
            }

            if (ixp+1 < blocks[1] && iyp+1 < blocks[0])
            {
                *(hist + (ixp+1)*blocks[0] + (iyp+1) + best_o*blocks[0]*blocks[1]) += vx0*vy0*v;
            }
        }
    }

    // compute energy in each block by summing over orientations
    for (int o = 0; o < 9; o++)
    {
        float *src1 = hist + o*blocks[0]*blocks[1];
        float *src2 = hist + (o+9)*blocks[0]*blocks[1];
        float *dst = norm;
        float *end = norm + blocks[1]*blocks[0];
        while (dst < end)
        {
            *(dst++) += (*src1 + *src2) * (*src1 + *src2);
            src1++;
            src2++;
        }
    }

   for (int y = 0; y < out[0]; y++)
   {
       for (int x = 0; x < out[1]; x++)
       {
           float *dst = hog_responses + y*(out[1]*out[2]) + x*out[2];
           float *src, *p, n1, n2, n3, n4;

           p = norm + (x+1)*blocks[0] + y+1;
           n1 = 1.0 / sqrt(*p + *(p+1) + *(p+blocks[0]) + *(p+blocks[0]+1) + eps);
           p = norm + (x+1)*blocks[0] + y;
           n2 = 1.0 / sqrt(*p + *(p+1) + *(p+blocks[0]) + *(p+blocks[0]+1) + eps);
           p = norm + x*blocks[0] + y+1;
           n3 = 1.0 / sqrt(*p + *(p+1) + *(p+blocks[0]) + *(p+blocks[0]+1) + eps);
           p = norm + x*blocks[0] + y;
           n4 = 1.0 / sqrt(*p + *(p+1) + *(p+blocks[0]) + *(p+blocks[0]+1) + eps);

           float t1 = 0;
           float t2 = 0;
           float t3 = 0;
           float t4 = 0;

           // contrast-sensitive features
           src = hist + (x+1)*blocks[0] + (y+1);
           for (int o = 0; o < 18; o++)
           {
               float h1 = hog_min(*src * n1, 0.2);
               float h2 = hog_min(*src * n2, 0.2);
               float h3 = hog_min(*src * n3, 0.2);
               float h4 = hog_min(*src * n4, 0.2);
               *dst = 0.5 * (h1 + h2 + h3 + h4);
               t1 += h1;
               t2 += h2;
               t3 += h3;
               t4 += h4;
               dst++;
               src += blocks[0]*blocks[1];
           }

           // contrast-insensitive features
           src = hist + (x+1)*blocks[0] + (y+1);
           for (int o = 0; o < 9; o++)
           {
               float sum = *src + *(src + 9*blocks[0]*blocks[1]);
               float h1 = hog_min(sum * n1, 0.2);
               float h2 = hog_min(sum * n2, 0.2);
               float h3 = hog_min(sum * n3, 0.2);
               float h4 = hog_min(sum * n4, 0.2);
               *dst = 0.5 * (h1 + h2 + h3 + h4);
               dst++;
               src += blocks[0]*blocks[1];
           }

           // texture hog_responsesures
           *dst = 0.2357 * t1;
           dst++;
           *dst = 0.2357 * t2;
           dst++;
           *dst = 0.2357 * t3;
           dst++;
           *dst = 0.2357 * t4;

           // truncation feature
           dst++;
           *dst = 0;
       }
   }

    delete[] hist;
    delete[] norm;
}

void Hog_responses::read(std::istream& in)
{
    using std::istringstream;

    const char* field_value;

    if(hog_responses != NULL)
    {
        delete[] hog_responses;
        hog_responses = NULL;
    }

    if (!(field_value = read_field_value(in, "bin_size")))
    {
        KJB_THROW_2(Illegal_argument, "Missing bin_size");
    }
    istringstream ist(field_value);
    ist >> bin_size;
    if (ist.fail() || (bin_size <= 0))
    {
        KJB_THROW_2(Illegal_argument, "Invalid line bin_size");
    }
    ist.clear(std::ios_base::goodbit);

    if (!(field_value = read_field_value(in, "num_rows")))
    {
        KJB_THROW_2(Illegal_argument, "Missing num_rows");
    }
    ist.str(field_value);
    ist >> num_rows;
    if (ist.fail() || (num_rows < 0))
    {
        KJB_THROW_2(Illegal_argument, "Invalid num_rows");
    }
    ist.clear(std::ios_base::goodbit);

    if (!(field_value = read_field_value(in, "num_cols")))
    {
        KJB_THROW_2(Illegal_argument, "Missing num_cols");
    }
    ist.str(field_value);
    ist >> num_cols;
    if (ist.fail() || (num_cols < 0))
    {
        KJB_THROW_2(Illegal_argument, "Invalid num_cols");
    }
    ist.clear(std::ios_base::goodbit);

    if (!(field_value = read_field_value(in, "hog_size")))
    {
        KJB_THROW_2(Illegal_argument, "Missing hog_size");
    }
    ist.str(field_value);
    ist >> hog_size;
    if (ist.fail() || (hog_size < 0))
    {
        KJB_THROW_2(Illegal_argument, "Invalid hog_size");
    }
    ist.clear(std::ios_base::goodbit);

    in.read((char *)hog_responses, sizeof(float)*bin_size);
}

/** @brief Writes this Line segment to an output stream. */
void Hog_responses::write(std::ostream& out) const
{
    out << " bin_size: " << bin_size << '\n'
        << " num_rows: " << num_rows << '\n'
        << " num_cols: " << num_cols << '\n'
        << " hog_size: " << hog_size << '\n';
    out.write((char *)hog_responses, sizeof(float)*bin_size);
}

void Hog_responses::get_HOG_picture(Image & positive_image, Image & negative_image, bool & negative_weights)
{
    std::vector<Matrix> bim;
    get_bims(bim);
    int bs = 20;

    int blocks[2];
    blocks[0] = (int)round((double)num_rows/(double)bin_size);
    blocks[1] = (int)round((double)num_cols/(double)bin_size);
    int size_1 = hog_max(blocks[0]-2, 0);
    int size_2 = hog_max(blocks[1]-2, 0);
    positive_image = Image::create_zero_image(size_1*bs, size_2*bs);
    negative_image = Image::create_zero_image(size_1*bs, size_2*bs);
    negative_weights = false;

    double hog_max = 0.0;

    Matrix temp_matrix;
    for(int i = 0; i < size_1; i++)
    {
        int iis = (i)*bs;
        for(int j = 0; j < size_2; j++)
        {
            int jjs = (j)*bs;
            for(int k = 0; k < 9; k++)
            {
                int hog_index = (i*size_2*32) + j*32 + k;
                temp_matrix = hog_responses[hog_index]*bim[k];
                if(hog_responses[hog_index] > 0.0)
                {
                    for(int internal_i = 0; internal_i < temp_matrix.get_num_rows(); internal_i++)
                    {
                        for(int internal_j = 0; internal_j < temp_matrix.get_num_rows(); internal_j++)
                        {
                            positive_image(iis + internal_i, jjs + internal_j, 0) += temp_matrix(internal_i, internal_j);
                        }
                    }
                }
                else if(hog_responses[hog_index] < 0.0)
                {
                    for(int internal_i = 0; internal_i < temp_matrix.get_num_rows(); internal_i++)
                    {
                        for(int internal_j = 0; internal_j < temp_matrix.get_num_rows(); internal_j++)
                        {
                            negative_image(iis + internal_i, jjs + internal_j, 0) -= temp_matrix(internal_i, internal_j);
                        }
                    }
                }
                if(fabs(hog_responses[hog_index]) > hog_max)
                {
                    hog_max = hog_responses[hog_index];
                }
            }
        }
    }

    if(hog_max == 0.0)
    {
        hog_max = DBL_EPSILON;
    }

    double scale_factor = 255.0/hog_max;
    for(int i = 0; i < positive_image.get_num_rows(); i++)
    {
        for(int j = 0; j < positive_image.get_num_cols(); j++)
        {
            positive_image(i, j, 0) *= scale_factor;
            positive_image(i, j, 1) = positive_image(i, j, 0);
            positive_image(i, j, 2) = positive_image(i, j, 0);
            negative_image(i, j, 0) *= scale_factor;
            negative_image(i, j, 1) = negative_image(i, j, 0);
            negative_image(i, j, 2) = negative_image(i, j, 0);
        }
    }
}

void Hog_responses::get_bims(std::vector<Matrix> & bim)
{
    bim.clear();
    int bs = 20;
    Matrix bim1(bs, bs, 0.0);
    int half_bs = round( ((double)bs)/2.0) - 1;
    for(int i = 0; i < bim1.get_num_rows(); i++)
    {
        bim1(i, half_bs) = 1.0;
        bim1(i, half_bs + 1) = 1.0;
    }
    bim.push_back(bim1);
    for(int i = 0; i < 8; i++)
    {
        bim.push_back(Matrix(bs, bs, 0.0));
    }

    bim[1](0, 12) = 1.0; bim[1](1, 12) = 1.0; bim[1](1, 13) = 1.0; bim[1](2, 12) = 1.0; bim[1](2, 13) = 1.0;
    bim[1](3, 11) = 1.0; bim[1](3, 12) = 1.0; bim[1](4, 11) = 1.0; bim[1](4, 12) = 1.0;
    bim[1](5, 11) = 1.0; bim[1](5, 12) = 1.0; bim[1](6, 10) = 1.0; bim[1](6, 11) = 1.0;
    bim[1](7, 10) = 1.0; bim[1](7, 11) = 1.0; bim[1](8, 9) = 1.0; bim[1](8, 10) = 1.0; bim[1](8, 11) = 1.0;
    bim[1](9, 9) = 1.0; bim[1](9, 10) = 1.0; bim[1](10, 9) = 1.0; bim[1](10, 10) = 1.0;
    bim[1](11, 8) = 1.0; bim[1](11, 9) = 1.0; bim[1](11, 10) = 1.0; bim[1](12, 8) = 1.0; bim[1](12, 9) = 1.0;
    bim[1](13, 8) = 1.0; bim[1](13, 9) = 1.0; bim[1](14, 7) = 1.0; bim[1](14, 8) = 1.0;
    bim[1](15, 7) = 1.0; bim[1](15, 8) = 1.0; bim[1](16, 7) = 1.0; bim[1](16, 8) = 1.0;
    bim[1](17, 6) = 1.0; bim[1](17, 7) = 1.0; bim[1](18, 6) = 1.0; bim[1](18, 7) = 1.0; bim[1](19, 7) = 1.0;

    bim[2](2, 15) = 1.0; bim[2](2, 16) = 1.0; bim[2](3, 14) = 1.0; bim[2](3, 15) = 1.0; bim[2](3, 16) = 1.0;
    bim[2](4, 13) = 1.0; bim[2](4, 14) = 1.0; bim[2](4, 15) = 1.0;
    bim[2](5, 12) = 1.0; bim[2](5, 13) = 1.0; bim[2](5, 14) = 1.0; bim[2](6, 12) = 1.0; bim[2](6, 13) = 1.0;
    bim[2](7, 11) = 1.0; bim[2](7, 12) = 1.0; bim[2](8, 10) = 1.0; bim[2](8, 11) = 1.0; bim[2](8, 12) = 1.0;
    bim[2](9, 9) = 1.0; bim[2](9, 10) = 1.0; bim[2](9, 11) = 1.0;
    bim[2](10, 8) = 1.0; bim[2](10, 9) = 1.0; bim[2](10, 10) = 1.0;
    bim[2](11, 7) = 1.0; bim[2](11, 8) = 1.0; bim[2](11, 9) = 1.0; bim[2](12, 7) = 1.0; bim[2](12, 8) = 1.0;
    bim[2](13, 6) = 1.0; bim[2](13, 7) = 1.0; bim[2](14, 5) = 1.0; bim[2](14, 6) = 1.0; bim[2](14, 7) = 1.0;
    bim[2](15, 4) = 1.0; bim[2](15, 5) = 1.0; bim[2](15, 6) = 1.0;
    bim[2](16, 3) = 1.0; bim[2](16, 4) = 1.0; bim[2](16, 5) = 1.0; bim[2](17, 3) = 1.0; bim[2](17, 4) = 1.0;

    bim[3](5, 18) = 1.0; bim[3](5, 17) = 1.0; bim[3](5, 16) = 1.0;
    bim[3](6, 17) = 1.0; bim[3](6, 16) = 1.0; bim[3](6, 15) = 1.0; bim[3](6, 14) = 1.0;
    bim[3](7, 12) = 1.0; bim[3](7, 13) = 1.0; bim[3](7, 14) = 1.0; bim[3](7, 15) = 1.0;
    bim[3](8, 11) = 1.0; bim[3](8, 12) = 1.0; bim[3](8, 13) = 1.0; bim[3](8, 12) = 1.0;
    bim[3](9, 9) = 1.0; bim[3](9, 10) = 1.0; bim[3](9, 11) = 1.0; bim[3](9, 12) = 1.0;
    bim[3](10, 8) = 1.0; bim[3](10, 9) = 1.0; bim[3](10, 10) = 1.0; bim[3](10, 11) = 1.0;
    bim[3](11, 5) = 1.0; bim[3](11, 6) = 1.0; bim[3](11, 7) = 1.0; bim[3](11, 8) = 1.0;
    bim[3](12, 4) = 1.0; bim[3](12, 5) = 1.0; bim[3](12, 6) = 1.0; bim[3](12, 7) = 1.0;
    bim[3](13, 2) = 1.0; bim[3](13, 3) = 1.0; bim[3](13, 4) = 1.0; bim[3](13, 5) = 1.0;
    bim[3](14, 1) = 1.0; bim[3](14, 2) = 1.0; bim[3](14, 3) = 1.0;

    bim[4](7, 19) = 1.0; bim[4](7, 18) = 1.0; bim[4](8, 19) = 1.0; bim[4](8, 18) = 1.0; bim[4](8, 17) = 1.0;
    bim[4](8, 16) = 1.0; bim[4](8, 15) = 1.0; bim[4](8, 14) = 1.0; bim[4](8, 13) = 1.0;
    bim[4](9, 18) = 1.0; bim[4](9, 17) = 1.0; bim[4](9, 16) = 1.0; bim[4](9, 15) = 1.0;
    bim[4](9, 14) = 1.0; bim[4](9, 13) = 1.0; bim[4](9, 12) = 1.0; bim[4](9, 11) = 1.0;
    bim[4](9, 10) = 1.0; bim[4](9, 9) = 1.0; bim[4](9, 8) = 1.0; bim[4](9, 7) = 1.0;
    bim[4](10, 12) = 1.0; bim[4](10, 11) = 1.0; bim[4](10, 10) = 1.0; bim[4](10, 9) = 1.0;
    bim[4](10, 8) = 1.0; bim[4](10, 7) = 1.0; bim[4](10, 6) = 1.0; bim[4](10, 5) = 1.0;
    bim[4](10, 4) = 1.0; bim[4](10, 3) = 1.0; bim[4](10, 2) = 1.0; bim[4](10, 1) = 1.0;
    bim[4](11, 6) = 1.0; bim[4](11, 5) = 1.0; bim[4](11, 4) = 1.0; bim[4](11, 3) = 1.0;
    bim[4](11, 2) = 1.0; bim[4](11, 1) = 1.0; bim[4](11, 0) = 1.0; bim[4](12, 1) = 1.0; bim[4](12, 0) = 1.0;

    bim[5](12, 19) = 1.0; bim[5](12, 18) = 1.0; bim[5](11, 19) = 1.0; bim[5](11, 18) = 1.0; bim[5](11, 17) = 1.0;
    bim[5](11, 16) = 1.0; bim[5](11, 15) = 1.0; bim[5](11, 14) = 1.0; bim[5](11, 13) = 1.0;
    bim[5](10, 18) = 1.0; bim[5](10, 17) = 1.0; bim[5](10, 16) = 1.0; bim[5](10, 15) = 1.0;
    bim[5](10, 14) = 1.0; bim[5](10, 13) = 1.0; bim[5](10, 12) = 1.0; bim[5](10, 11) = 1.0;
    bim[5](10, 10) = 1.0; bim[5](10, 9) = 1.0; bim[5](10, 8) = 1.0; bim[5](10, 7) = 1.0;
    bim[5](9, 12) = 1.0; bim[5](9, 11) = 1.0; bim[5](9, 10) = 1.0; bim[5](9, 9) = 1.0;
    bim[5](9, 8) = 1.0; bim[5](9, 7) = 1.0; bim[5](9, 6) = 1.0; bim[5](9, 5) = 1.0;
    bim[5](9, 4) = 1.0; bim[5](9, 3) = 1.0; bim[5](9, 2) = 1.0; bim[5](9, 1) = 1.0;
    bim[5](8, 6) = 1.0; bim[5](8, 5) = 1.0; bim[5](8, 4) = 1.0; bim[5](8, 3) = 1.0;
    bim[5](8, 2) = 1.0; bim[5](8, 1) = 1.0; bim[5](8, 0) = 1.0; bim[5](7, 1) = 1.0; bim[5](7, 0) = 1.0;

    bim[6](14, 18) = 1.0; bim[6](14, 17) = 1.0; bim[6](14, 16) = 1.0;
    bim[6](13, 17) = 1.0; bim[6](13, 16) = 1.0; bim[6](13, 15) = 1.0; bim[6](13, 14) = 1.0;
    bim[6](12, 12) = 1.0; bim[6](12, 13) = 1.0; bim[6](12, 14) = 1.0; bim[6](12, 15) = 1.0;
    bim[6](11, 11) = 1.0; bim[6](11, 12) = 1.0; bim[6](11, 13) = 1.0; bim[6](11, 12) = 1.0;
    bim[6](10, 9) = 1.0; bim[6](10, 10) = 1.0; bim[6](10, 11) = 1.0; bim[6](10, 12) = 1.0;
    bim[6](9, 8) = 1.0; bim[6](9, 9) = 1.0; bim[6](9, 10) = 1.0; bim[6](9, 11) = 1.0;
    bim[6](8, 5) = 1.0; bim[6](8, 6) = 1.0; bim[6](8, 7) = 1.0; bim[6](8, 8) = 1.0;
    bim[6](7, 4) = 1.0; bim[6](7, 5) = 1.0; bim[6](7, 6) = 1.0; bim[6](7, 7) = 1.0;
    bim[6](6, 2) = 1.0; bim[6](6, 3) = 1.0; bim[6](6, 4) = 1.0; bim[6](6, 5) = 1.0;
    bim[6](5, 1) = 1.0; bim[6](5, 2) = 1.0; bim[6](5, 3) = 1.0;

    bim[7](17, 15) = 1.0; bim[7](17, 16) = 1.0; bim[7](16, 14) = 1.0; bim[7](16, 15) = 1.0; bim[7](16, 16) = 1.0;
    bim[7](15, 13) = 1.0; bim[7](15, 14) = 1.0; bim[7](15, 15) = 1.0;
    bim[7](14, 12) = 1.0; bim[7](14, 13) = 1.0; bim[7](14, 14) = 1.0; bim[7](13, 12) = 1.0; bim[7](13, 13) = 1.0;
    bim[7](12, 11) = 1.0; bim[7](12, 12) = 1.0; bim[7](11, 10) = 1.0; bim[7](11, 11) = 1.0; bim[7](11, 12) = 1.0;
    bim[7](10, 9) = 1.0; bim[7](10, 10) = 1.0; bim[7](10, 11) = 1.0;
    bim[7](9, 8) = 1.0; bim[7](9, 9) = 1.0; bim[7](9, 10) = 1.0;
    bim[7](8, 7) = 1.0; bim[7](8, 8) = 1.0; bim[7](8, 9) = 1.0; bim[7](7, 7) = 1.0; bim[7](7, 8) = 1.0;
    bim[7](6, 6) = 1.0; bim[7](6, 7) = 1.0; bim[7](5, 5) = 1.0; bim[7](5, 6) = 1.0; bim[7](5, 7) = 1.0;
    bim[7](4, 4) = 1.0; bim[7](4, 5) = 1.0; bim[7](4, 6) = 1.0;
    bim[7](3, 3) = 1.0; bim[7](3, 4) = 1.0; bim[7](3, 5) = 1.0; bim[7](2, 3) = 1.0; bim[7](2, 4) = 1.0;

    bim[8](19, 12) = 1.0; bim[8](18, 12) = 1.0; bim[8](18, 13) = 1.0; bim[8](17, 12) = 1.0; bim[8](17, 13) = 1.0;
    bim[8](16, 11) = 1.0; bim[8](16, 12) = 1.0; bim[8](15, 11) = 1.0; bim[8](15, 12) = 1.0;
    bim[8](14, 11) = 1.0; bim[8](14, 12) = 1.0; bim[8](13, 10) = 1.0; bim[8](13, 11) = 1.0;
    bim[8](12, 10) = 1.0; bim[8](12, 11) = 1.0; bim[8](11, 9) = 1.0; bim[8](11, 10) = 1.0; bim[8](11, 11) = 1.0;
    bim[8](10, 9) = 1.0; bim[8](10, 10) = 1.0; bim[8](9, 9) = 1.0; bim[8](9, 10) = 1.0;
    bim[8](8, 8) = 1.0; bim[8](8, 9) = 1.0; bim[8](8, 10) = 1.0; bim[8](7, 8) = 1.0; bim[8](7, 9) = 1.0;
    bim[8](6, 8) = 1.0; bim[8](6, 9) = 1.0; bim[8](5, 7) = 1.0; bim[8](5, 8) = 1.0;
    bim[8](4, 7) = 1.0; bim[8](4, 8) = 1.0; bim[8](3, 7) = 1.0; bim[8](3, 8) = 1.0;
    bim[8](2, 6) = 1.0; bim[8](2, 7) = 1.0; bim[8](1, 6) = 1.0; bim[8](1, 7) = 1.0; bim[8](0, 7) = 1.0;
}



void kjb::rotate_matrix_90_degrees(Matrix & m, int number_of_times)
{
    number_of_times = number_of_times % 4;
    if(number_of_times < 0)
    {
        number_of_times += 4;
    }

    switch(number_of_times)
    {
    case 0:
        //Nothing to do here
        break;
    case 1:
        m = m.transpose();
        flip_matrix_ud(m);
        break;
    case 2:
        flip_matrix_lr(m);
        flip_matrix_ud(m);
        break;
    case 3:
        flip_matrix_ud(m);
        m = m.transpose();
        break;
    }
}
