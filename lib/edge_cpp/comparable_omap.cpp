/**
 * @file  Comparable_omap.cpp
 *
 * @author Josh Bowdish
 *
 * @brief - creates an omap from an image that can call compare_omap(compareto)
 *  to see how similar it is.
 */

#include "edge_cpp/comparable_omap.h"
#include <iostream>

using namespace kjb;

#define COLOR_TOLERANCE 5

#define OMAP_HORIZONTAL 0
#define OMAP_VERTICAL_1 1
#define OMAP_VERTICAL_2 2
#define OMAP_BLACK 3


void Comparable_omap::prepare_int_maps(const kjb::Image & base)
{
    map_base.zero_out(_num_rows, _num_cols);
    accum_hor.zero_out(_num_rows, _num_cols);
    accum_ver1.zero_out(_num_rows, _num_cols);
    accum_ver2.zero_out(_num_rows, _num_cols);

    for(unsigned int i = 0; i < base.get_num_rows(); i++)
    {
        for(unsigned int j = 0; j < base.get_num_cols(); j++)
        {
            if(base(i,j,0) > 250)
            {
                map_base(i, j) = OMAP_HORIZONTAL;
                assert(base(i,j,1) < COLOR_TOLERANCE);
                assert(base(i,j,2) < COLOR_TOLERANCE);
            }
            else if(base(i,j,1) > 250)
            {
                map_base(i, j) = OMAP_VERTICAL_1;
                assert(base(i,j,0) < COLOR_TOLERANCE);
                assert(base(i,j,2) < COLOR_TOLERANCE);
            }
            else if(base(i,j,2) > 250)
            {
                map_base(i, j) = OMAP_VERTICAL_2;
                assert(base(i,j,0) < COLOR_TOLERANCE);
                assert(base(i,j,1) < COLOR_TOLERANCE);
            }
            else
            {
                map_base(i, j) = OMAP_BLACK;
            }
        }
    }
    unsigned int prev_col = 0;
    unsigned int prev_row = 0;
    for(unsigned int i = 0; i < base.get_num_rows(); i++)
    {
        if(i > 0)
        {
            prev_col = base.get_num_cols() -1;
            prev_row = i - 1;
            accum_hor(i, 0) = accum_hor(prev_row, prev_col);
            accum_ver1(i, 0) = accum_ver1(prev_row, prev_col);
            accum_ver2(i, 0) = accum_ver2(prev_row, prev_col);
            switch(map_base(i, 0))
            {
                case OMAP_HORIZONTAL:
                    accum_hor(i, 0)++;
                    break;
                case OMAP_VERTICAL_1:
                    accum_ver1(i, 0)++;
                    break;
                case OMAP_VERTICAL_2:
                    accum_ver2(i, 0)++;
                    break;
                default:
                    break;
            }
        }
        else
        {
            switch(map_base(0, 0))
            {
                case OMAP_HORIZONTAL:
                    accum_hor(0, 0)++;
                    break;
                case OMAP_VERTICAL_1:
                    accum_ver1(0, 0)++;
                    break;
                case OMAP_VERTICAL_2:
                    accum_ver2(0, 0)++;
                    break;
                default:
                    break;
            }
        }
        for(unsigned int j = 1; j < base.get_num_cols(); j++)
        {
            accum_hor(i, j) = accum_hor(i, j-1);
            accum_ver1(i, j) = accum_ver1(i, j-1);
            accum_ver2(i, j) = accum_ver2(i, j-1);
            switch(map_base(i, j))
            {
                case OMAP_HORIZONTAL:
                    accum_hor(i, j)++;
                    break;
                case OMAP_VERTICAL_1:
                    accum_ver1(i, j)++;
                    break;
                case OMAP_VERTICAL_2:
                    accum_ver2(i, j)++;
                    break;
                default:
                    break;
            }
        }
    }
}

/*double Comparable_omap::compare_omap(const Image & compareto) const
{
    //totalbaseblack = get_black(base);
    //totalcompareblack = get_black(compareto);
    
    double firstpass=0;
    double secondpass=0;

    firstpass = match_pixels(compareto, base);
    secondpass = match_pixels(compareto, reversedbase);
    
    if(firstpass>secondpass)
    {
         return firstpass;
    }
    else
    {
       //std::cout << "Better match made with sides reversed\n" << std::endl;
       return secondpass;   
    }
}*/

/*double  Comparable_omap::match_pixels
(
    const Image  & basein,
    const Image  & compareto
) const
{
    int totalright=0;

    if(basein.get_num_rows()!=compareto.get_num_rows()){
        printf("unequal rows\n");
        return 0;
    }

    if(basein.get_num_cols()!=compareto.get_num_cols()){
        printf("unequal columns\n");
        return 0;
    }

    for(int i = 0; i < basein.get_num_rows(); i++)
    {
        for(int j = 0; j < basein.get_num_cols(); j++)
        {
            if(
                (fabs(compareto(i, j, 0) - basein(i,j,0)) < COLOR_TOLERANCE) &&//red
                (fabs(compareto(i, j, 1) - basein(i,j,1)) < COLOR_TOLERANCE) &&//green
                (fabs(compareto(i, j, 2) - basein(i,j,2)) < COLOR_TOLERANCE) )
            {
                if( ! ((compareto(i, j, 0) < COLOR_TOLERANCE) && (compareto(i,j, 1) < COLOR_TOLERANCE) && (compareto(i, j, 2) < COLOR_TOLERANCE) ) )
                {
                    totalright++;
                }
            }
        }
    }


    return totalright/ ((double) total - totalbaseblack);
}//match_pixels */


double Comparable_omap::compare_omap
(
    const Int_matrix  & imap
) const
{
    int totalright = 0;
    int totalright2 = 0;

    if(map_base.get_num_rows() != imap.get_num_rows())
    {
        std::cout << map_base.get_num_rows() << "  " << imap.get_num_rows() << std::endl;
        printf("unequal rows\n");
        return 0;
    }

    if(map_base.get_num_cols() != imap.get_num_cols())
    {
        std::cout << map_base.get_num_cols() << "  " << imap.get_num_cols() << std::endl;
        printf("unequal columns\n");
        return 0;
    }

    for(int i = 0; i < imap.get_num_rows(); i++)
    {
        for(int j = 0; j < imap.get_num_cols(); j++)
        {
            if(map_base(i, j) == OMAP_BLACK)
            {
                continue;
            }
            int face = imap(i, j);
            face = (face>>8)&0x07;
            if(face < 2)
            {
                 if(map_base(i, j) == OMAP_VERTICAL_1)
                 {
                    totalright++;
                 }
                 else if(map_base(i, j) == OMAP_VERTICAL_2)
                 {
                     totalright2++;
                 }
            }
            else if(face < 4)
            {
                //Horizontal surfaces
                if(map_base(i, j) == OMAP_HORIZONTAL)
                {
                    totalright++;
                    totalright2++;
                }
            }
            else if(face < 6)
            {
                if(map_base(i, j) == OMAP_VERTICAL_2)
                {
                    totalright++;
                }
                else if(map_base(i, j) == OMAP_VERTICAL_1)
                {
                    totalright2++;
                }
            }
            else
            {
                std::cout << "BLA BLA" << std::endl;
            }
            // 0 and 1 are parallel, 2 and 3 are the horizontal walls, 4 and 5 are parallel

        }
    }

    if(totalright2 > totalright)
    {
        totalright = totalright2;
    }

    return totalright/ ((double) total - totalbaseblack);
}//match_pixels


double Comparable_omap::compare_omap_integral
(
    const std::vector<kjb::Int_vector> & surface_changes,
    int surface_counter
) const
{
    int totalright = 0;
    int totalright2 = 0;
    int diff = 0;

    if(surface_changes.size() == 0)
    {
        return 0.0000000001;
    }

    if(surface_changes[0](2) < 2)
    {
        totalright += (accum_ver1(surface_changes[0](0), surface_changes[0](1)));
        totalright2 += (accum_ver2(surface_changes[0](0), surface_changes[0](1)));
    }
    else if(surface_changes[0](2) < 4)
    {
        //Horizontal surfaces
        diff = (accum_hor(surface_changes[0](0), surface_changes[0](1)));
        totalright += diff;
        totalright2 += diff;
    }
    else if(surface_changes[0](2) < 6)
    {
        totalright2 += (accum_ver1(surface_changes[0](0), surface_changes[0](1)));
        totalright += (accum_ver2(surface_changes[0](0), surface_changes[0](1)));
    }
    unsigned int start_row = surface_changes[0](0);
    unsigned int start_col = surface_changes[0](1);

    for(unsigned int i = 1; i < surface_counter; i++)
    {
        if(surface_changes[i](2) < 2)
        {
            totalright += (accum_ver1(surface_changes[i](0), surface_changes[i](1))) - (accum_ver1(start_row, start_col));
            totalright2 += (accum_ver2(surface_changes[i](0), surface_changes[i](1))) - (accum_ver2(start_row, start_col));
        }
        else if(surface_changes[i](2) < 4)
        {
            //Horizontal surfaces
            diff = (accum_hor(surface_changes[i](0), surface_changes[i](1))) - (accum_hor(start_row, start_col));
            totalright += diff;
            totalright2 += diff;
        }
        else if(surface_changes[i](2) < 6)
        {
            totalright2 += (accum_ver1(surface_changes[i](0), surface_changes[i](1))) - (accum_ver1(start_row, start_col));
            totalright += (accum_ver2(surface_changes[i](0), surface_changes[i](1))) - (accum_ver2(start_row, start_col));
        }
        // 0 and 1 are parallel, 2 and 3 are the horizontal walls, 4 and 5 are parallel
        start_row = surface_changes[i](0);
        start_col = surface_changes[i](1);
    }

    if(totalright2 > totalright)
    {
        totalright = totalright2;
    }

    return totalright/ ((double) total - totalbaseblack);
}//match_pixels



/*
* get_black(Image&) - tells you how many pixels in the image are black.
*/
int Comparable_omap::get_black(const Image & howblack){
    
    int thisblack=0;

    for(int i=0;i<howblack.get_num_rows();i++){
        for(int j=0;j<howblack.get_num_cols();j++){
            if(
            howblack(i, j, 0)< COLOR_TOLERANCE &&//red
            howblack(i, j, 1)< COLOR_TOLERANCE &&//green
            howblack(i, j, 2)< COLOR_TOLERANCE){//blue
                thisblack++;
            }
        }
    }
    return thisblack;
}//get_black

/*void Comparable_omap::make_opposite_omap()
{
    reversedbase = base;
    for(int i=0;i<base.get_num_rows();i++){
        for(int j=0;j<base.get_num_cols();j++){
            if( (base(i,j,1) > COLOR_TOLERANCE ) && (base(i,j,2) > COLOR_TOLERANCE) )
            {
                KJB_THROW_2(KJB_error, "Invalid orientation map");
            }
            if((base(i,j,1) > COLOR_TOLERANCE) )
            {
                if((base(i,j,0) > COLOR_TOLERANCE) )
                {
                    KJB_THROW_2(KJB_error, "Invalid orientation map");
                }
                reversedbase(i,j,1) = 0.0;
                reversedbase(i,j,2) = 255.0;
            }
            else if((base(i,j,2) > COLOR_TOLERANCE) )
            {
                if((base(i,j,0) > COLOR_TOLERANCE) )
                {
                    KJB_THROW_2(KJB_error, "Invalid orientation map");
                }
                reversedbase(i,j,1) = 255.0;
                reversedbase(i,j,2) = 0.0;
            }

        }
    }

}//make_opposite_omap */

void Comparable_omap::convert_map_to_image
(
    const Int_matrix & imap,
    Image & img
)
{
    //img.create_zero_image(imap.get_num_rows(), imap.get_num_cols());
    for(unsigned int i = 0; i < imap.get_num_rows(); i++)
    {
        for(unsigned int j = 0; j < imap.get_num_cols(); j++)
        {
            int face = imap(i, j);
            face = (face>>8)&0x07;
            if(face < 2)
            {
                img(i, j, 0) = 0.0;
                img(i, j, 1) = 255.0;
                img(i, j, 2) = 0.0;

            }
            else if(face < 4)
            {
                img(i, j, 0) = 255.0;
                img(i, j, 1) = 0.0;
                img(i, j, 2) = 0.0;
            }
            else
            {
                img(i, j, 0) = 0.0;
                img(i, j, 1) = 0.0;
                img(i, j, 2) = 255.0;
            }

        }
    }
}


