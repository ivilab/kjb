
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

#include "st_cpp/st_manhattan_hog.h"
#include <cmath>
#include <iostream>

using namespace kjb;

Manhattan_hog::Manhattan_hog
(
    const Hog_responses & hog_responses,
    const Image & image,
    const Perspective_camera & camera,
    const Parametric_parapiped & parapiped
) : Readable(), Writeable()
{
    manhattan_hog_size = compute_manhattan_hog_size(hog_responses.get_hog_size());
    std::cout << "Man hog size:" << manhattan_hog_size <<std::endl;
    hog_responses_horizontal = new float[manhattan_hog_size];
    hog_responses_vertical_1 = new float[manhattan_hog_size];
    hog_responses_vertical_2 = new float[manhattan_hog_size];

    //double princ_x = hog_responses.get_num_cols()/2.0;
    //double princ_y = hog_responses.get_num_rows()/2.0;
    double princ_x = (image.get_num_cols()/2.0) + camera.get_principal_point_x();
    double princ_y = (image.get_num_rows()/2.0) - camera.get_principal_point_y();
    if(camera.get_focal_length() <= DBL_EPSILON)
    {
        KJB_THROW_2(Illegal_argument,"Bad value for focal length");
    }

    hog_maxs.zero_out(3);

    num_hog_cols = hog_responses.get_hog_num_cols();
    num_hog_rows = hog_responses.get_hog_num_rows();
    bin_size = hog_responses.get_bin_size();
    int num_angle_bins = 9;
    std::vector<kjb::Vector> manhattan_dir_3d_horizontal;
    std::vector<kjb::Vector> manhattan_dir_3d_vertical1;
    std::vector<kjb::Vector> manhattan_dir_3d_vertical2;
    std::cout << "Man 3D manhattan dirs" <<std::endl;
    get_3D_manhattan_directions(manhattan_dir_3d_horizontal, manhattan_dir_3d_vertical1,
            manhattan_dir_3d_vertical2, num_angle_bins);
    std::vector<kjb::Vector> projected_horizontal;
    std::vector<kjb::Vector> projected_vertical1;
    std::vector<kjb::Vector> projected_vertical2;

    camera.prepare_for_rendering(true);
    double z_distance = 10.0;
    kjb::Vector position_3D(4, 1.0);
    position_3D(2) = -camera.get_focal_length();
    kjb::Vector position_3D_world;
    kjb::Vector position_3D_in_pp;
    const float * hog_responses_2D = hog_responses.get_hog_responses();
    double angle_increment = M_PI / ((double) (num_angle_bins));
    for(int  i = 0; i < num_hog_rows; i++)
    {
          for(int  j = 0; j < num_hog_cols; j++)
          {
              position_3D(0) = (hog_responses.get_hog_cell_x_center(j) - princ_x);
              position_3D(1) = -(hog_responses.get_hog_cell_y_center(i) - princ_y);
              camera.get_point_in_world_coordinates(position_3D, position_3D_world);
              parapiped.get_point_in_parapiped_coordinates(position_3D_world, position_3D_in_pp);
              get_projected_manhattan_directions(manhattan_dir_3d_horizontal, manhattan_dir_3d_vertical1,
              manhattan_dir_3d_vertical2, position_3D_in_pp, parapiped, camera, princ_y,
              projected_horizontal, projected_vertical1, projected_vertical2);
              int input_offset = i*(num_hog_cols*32) + j*32;
              int ooffset = i*(num_hog_cols*9) + j*9;
              const float * input_ptr = hog_responses_2D + input_offset;
              interpolate_projected_directions(projected_horizontal, input_ptr, hog_responses_horizontal + ooffset, angle_increment, num_angle_bins);
              interpolate_projected_directions(projected_vertical1, input_ptr, hog_responses_vertical_1 + ooffset, angle_increment, num_angle_bins);
              interpolate_projected_directions(projected_vertical2, input_ptr, hog_responses_vertical_2 + ooffset, angle_increment, num_angle_bins);
              for(unsigned int k = 0; k < 9; k++)
              {
                  if(hog_responses_2D[input_offset + k] > hog_maxs(0))
                  {
                      hog_maxs(0) = hog_responses_2D[input_offset + k];
                  }
                  if(hog_responses_2D[input_offset + k + 9] > hog_maxs(1))
                  {
                      hog_maxs(1) = hog_responses_2D[input_offset + k + 9];
                  }
                  if(hog_responses_2D[input_offset + k + 18] > hog_maxs(2))
                  {
                      hog_maxs(2) = hog_responses_2D[input_offset + k + 18];
                  }
              }
              //break;
          }
          //break;
    }
}

void Manhattan_hog::read(std::istream& in)
{
    using std::istringstream;

    const char* field_value;

    /*if (!(field_value = read_field_value(in, "bin_size")))
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
    ist.clear(std::ios_base::goodbit);*/
}

/** @brief Writes this Line segment to an output stream. */
void Manhattan_hog::write(std::ostream& out) const
{

}

void Manhattan_hog::find_2D_hog_directions(std::vector<kjb::Vector> & directions_2D, int num_angle_bins) const
{
    Vector initial_vector(2, 0.0);
    initial_vector(1) = 1.0;
    directions_2D.clear();
    directions_2D.push_back(initial_vector);
    double angle_increment = M_PI / ((double) (num_angle_bins - 1));
    for(int i = 1; i < num_angle_bins; i++)
    {
        directions_2D.push_back(initial_vector*Matrix::create_2d_rotation_matrix(-((double)i) * angle_increment));
    }
}

double Manhattan_hog::find_2D_weight_by_interpolation
(
    const Vector & idirection,
    double angle_increment,
    int inum_bins,
    const float * weights
) const
{
    //std::cout << "The direction:" << idirection << std::endl;
    double idirection_norm = sqrt(idirection(0)*idirection(0) + idirection(1)*idirection(1));
    double angle = acos(idirection(1)/idirection_norm);
    if(idirection(0) > 0.0)
    {
        angle = -angle;
    }
    //std::cout << "The angle is:" << angle << std::endl;

    if(angle > 0.0)
    {
        angle = M_PI - angle;
    }
    else
    {
        angle = fabs(angle);
    }
    //std::cout << "The angle after transformation is is:" << angle << std::endl;
    //std::cout << "The angle increment is:" << angle_increment << std::endl;
    int left_2D_vector = std::floor(angle/angle_increment);
    int right_2D_vector = left_2D_vector + 1;
    if(right_2D_vector >= (inum_bins))
    {
        right_2D_vector = 0;
    }
    /*std::cout << "The interpol ones:" << weights[0] << "    " << weights[8] << std::endl << std::endl << std::endl;
    std::cout << "The left - right bins are: " << left_2D_vector << "  - " << right_2D_vector << std::endl;
    std::cout << "The interpol ones for real:" << weights[left_2D_vector] << "    " << weights[right_2D_vector] << std::endl; */
    double ratio_1 = fmod(angle, angle_increment)/angle_increment;
    return (weights[left_2D_vector]*(1 - ratio_1)) + (weights[right_2D_vector]*(ratio_1));
}

void Manhattan_hog::get_3D_manhattan_directions
(
    std::vector<kjb::Vector> & manhattan_dir_3d_horizontal,
    std::vector<kjb::Vector> & manhattan_dir_3d_vertical1,
    std::vector<kjb::Vector> & manhattan_dir_3d_vertical2,
    int num_directions
) const
{
    manhattan_dir_3d_horizontal.clear();
    manhattan_dir_3d_vertical1.clear();
    manhattan_dir_3d_vertical2.clear();
    double angle_increment = M_PI/((double) num_directions);
    Vector start_horizontal(3, 0.0);
    start_horizontal(2) = 1.0;
    Vector start_vertical1(3, 0.0);
    start_vertical1(1) = 1.0;
    Vector start_vertical2(3, 0.0);
    start_vertical2(1) = 1.0;
    manhattan_dir_3d_horizontal.push_back(start_horizontal);
    manhattan_dir_3d_vertical1.push_back(start_vertical1);
    manhattan_dir_3d_vertical2.push_back(start_vertical2);
    //std::cout << "Angle increment:" << angle_increment << std::endl;

    for(int i = 1; i < num_directions; i++)
    {
        manhattan_dir_3d_horizontal.push_back(start_horizontal*Matrix::create_3d_rotation_matrix(-((double)i)*angle_increment, 0.0, 1.0, 0.0));
        manhattan_dir_3d_vertical1.push_back(start_vertical1*Matrix::create_3d_rotation_matrix(-((double)i)*angle_increment, 1.0, 0.0, 0.0));;
        manhattan_dir_3d_vertical2.push_back(start_vertical2*Matrix::create_3d_rotation_matrix(-((double)i)*angle_increment, 0.0, 0.0, 1.0));
        //std::cout << "3D dir:" << manhattan_dir_3d_vertical2[i] << std::endl;
        //std::cout << "Euler rotation matrix:" << std::endl << Matrix::create_euler_rotation_matrix(0.0, -((double)i)*angle_increment, 0.0) << std::endl;
    }

    std::cout << "DIRS 1:" << std::endl;
    for(int i = 0; i < num_directions; i++)
    {
        std::cout << manhattan_dir_3d_horizontal[i] << std::endl;
    }
    std::cout << "DIRS 2:" << std::endl;
    for(int i = 0; i < num_directions; i++)
    {
        std::cout << manhattan_dir_3d_vertical1[i] << std::endl;
    }
    std::cout << "DIRS 3:" << std::endl;
    for(int i = 0; i < num_directions; i++)
    {
        std::cout << manhattan_dir_3d_vertical2[i] << std::endl;
    }
}

void Manhattan_hog::get_projected_manhattan_directions
(
    const std::vector<kjb::Vector> & manhattan_dir_3d_horizontal,
    const std::vector<kjb::Vector> & manhattan_dir_3d_vertical1,
    const std::vector<kjb::Vector> & manhattan_dir_3d_vertical2,
    const kjb::Vector & center_in_pp_coordinates,
    const kjb::Parametric_parapiped & pp,
    const kjb::Perspective_camera & camera,
    int img_height,
    std::vector<kjb::Vector> & projected_manhattan_dir_horizontal,
    std::vector<kjb::Vector> & projected_manhattan_dir_vertical1,
    std::vector<kjb::Vector> & projected_manhattan_dir_vertical2
) const
{
    projected_manhattan_dir_horizontal.clear();
    projected_manhattan_dir_vertical1.clear();
    projected_manhattan_dir_vertical2.clear();
    int num_directions = manhattan_dir_3d_horizontal.size();
    kjb::Vector direction_3Dpp(4, 1.0);
    kjb::Vector direction_3Dworld(3, 0.0);
    kjb::Vector center_3Dworld(3, 0.0);
    kjb::Vector temp_2D_dir(2, 0.0);
    double _x, _y, _z;
    double _xc, _yc, _zc;

    pp.get_point_in_world_coordinates(center_in_pp_coordinates, center_3Dworld);
    camera.get_rendering_interface().project_point(_xc, _yc, _zc, center_3Dworld, (double)img_height);

    for(int i = 0; i < num_directions; i++)
    {
        for(int k = 0; k < 3; k++)
        {
            direction_3Dpp(k) = center_in_pp_coordinates(k) + manhattan_dir_3d_horizontal[i](k);
        }
        pp.get_point_in_world_coordinates(direction_3Dpp, direction_3Dworld);
        camera.get_rendering_interface().project_point(_x, _y, _z, direction_3Dworld, (double)img_height);
        temp_2D_dir(0) = _x - _xc;
        temp_2D_dir(1) = _yc - _y;
        projected_manhattan_dir_horizontal.push_back(temp_2D_dir);

        for(int k = 0; k < 3; k++)
        {
            direction_3Dpp(k) = center_in_pp_coordinates(k) + manhattan_dir_3d_vertical1[i](k);
        }
        pp.get_point_in_world_coordinates(direction_3Dpp, direction_3Dworld);
        camera.get_rendering_interface().project_point(_x, _y, _z, direction_3Dworld, (double)img_height);
        temp_2D_dir(0) = _x - _xc;
        temp_2D_dir(1) = _yc - _y;
        projected_manhattan_dir_vertical1.push_back(temp_2D_dir);

        for(int k = 0; k < 3; k++)
        {
            direction_3Dpp(k) = center_in_pp_coordinates(k) + manhattan_dir_3d_vertical2[i](k);
        }
        pp.get_point_in_world_coordinates(direction_3Dpp, direction_3Dworld);
        camera.get_rendering_interface().project_point(_x, _y, _z, direction_3Dworld, (double)img_height);
        temp_2D_dir(0) = _x - _xc;
        temp_2D_dir(1) = _yc - _y;
        projected_manhattan_dir_vertical2.push_back(temp_2D_dir);
    }
}

void Manhattan_hog::interpolate_projected_directions
(
    const std::vector<kjb::Vector> & projected_directions,
    const float * weights_2D,
    float * weights_3D,
    double angle_increment,
    int inum_bins
)
{
    for(int i = 0; i < projected_directions.size(); i++)
    {
        weights_3D[i] = find_2D_weight_by_interpolation(projected_directions[i], angle_increment, inum_bins, weights_2D);
        //std::cout << "The weight is:" << weights_3D[i] << std::endl;
    }
}

void Manhattan_hog::get_line_segments_for_drawing
(
    std::vector<Line_segment> & line_segments,
    const std::vector<kjb::Vector> & projected_directions,
    double center_x,
    double center_y,
    double length
) const
{
    line_segments.clear();
    for(int i = 0; i < projected_directions.size(); i++)
    {
        //std::cout << "Projected directions is:" << projected_directions[i] << std::endl;
        double direction_norm = sqrt(projected_directions[i](0)*projected_directions[i](0) +
                projected_directions[i](1)*projected_directions[i](1));
        //std::cout << "Projected directions is:" << projected_directions[i]/direction_norm << std::endl;
        double angle = acos(projected_directions[i](1)/direction_norm);
        if(projected_directions[i](0) > 0.0)
        {
            angle = -angle;
        }
        //std::cout << "The angle is:" << angle << std::endl;
        angle += M_PI_2;
        //std::cout << "The transformed angle is:" << angle << std::endl;
        line_segments.push_back(Line_segment(center_x, center_y, angle, length));
        //break;
    }
}

Image Manhattan_hog::draw_image_with_vertical_segments
(
    const Hog_responses & hog_responses,
    const Image & image,
    const Perspective_camera & camera,
    const Parametric_parapiped & parapiped
) const
{
    int length = 20.0;
    int num_angle_bins = 9;
    Image img = Image::create_zero_image(num_hog_rows*length, num_hog_cols*length);
    std::vector<Line_segment> line_segments;

    //double princ_x = image.get_num_cols()/2.0;
    //double princ_y = image.get_num_rows()/2.0;

    double princ_x = (image.get_num_cols()/2.0) + camera.get_principal_point_x();
    double princ_y = (image.get_num_rows()/2.0) - camera.get_principal_point_y();
    if(camera.get_focal_length() <= DBL_EPSILON)
    {
        KJB_THROW_2(Illegal_argument,"Bad value for focal length");
    }

    std::vector<kjb::Vector> manhattan_dir_3d_horizontal;
    std::vector<kjb::Vector> manhattan_dir_3d_vertical1;
    std::vector<kjb::Vector> manhattan_dir_3d_vertical2;
    get_3D_manhattan_directions(manhattan_dir_3d_horizontal, manhattan_dir_3d_vertical1,
    manhattan_dir_3d_vertical2, num_angle_bins);
    std::vector<kjb::Vector> projected_horizontal;
    std::vector<kjb::Vector> projected_vertical1;
    std::vector<kjb::Vector> projected_vertical2;

    double z_distance = 50.0;
    kjb::Vector position_3D(4, 1.0);
    position_3D(2) = -camera.get_focal_length();
    kjb::Vector position_3D_world;
    kjb::Vector position_3D_in_pp;
    const float * hog_responses_2D = hog_responses.get_hog_responses();
    for(int  i = 0; i < num_hog_rows; i++)
    {
          //std::cout << "Center x: " << 10.0 + j *20.0 << std::endl;
          for(int  j = 0; j < num_hog_cols; j++)
          {
              if( !((i == 59) && (j == 25)))
              {
                  //continue;
              }
              //std::cout << "Image position: " << hog_responses.get_hog_cell_x_center(j) << "  " << hog_responses.get_hog_cell_y_center(i) << std::endl;
              position_3D(0) = (hog_responses.get_hog_cell_x_center(j) - princ_x);
              position_3D(1) = -(hog_responses.get_hog_cell_y_center(i) - princ_y);
              //std::cout << "Position 3D: " << position_3D << std::endl;
              camera.get_point_in_world_coordinates(position_3D, position_3D_world);
              parapiped.get_point_in_parapiped_coordinates(position_3D_world, position_3D_in_pp);
              get_projected_manhattan_directions(manhattan_dir_3d_horizontal, manhattan_dir_3d_vertical1,
              manhattan_dir_3d_vertical2, position_3D_in_pp, parapiped, camera, princ_y,
              projected_horizontal, projected_vertical1, projected_vertical2);
              /*std::cout << "The direction is is:" << projected_vertical1[0] << std::endl;
              double direction_norm = sqrt(projected_vertical1[0](0)*projected_vertical1[0](0)
                      + projected_vertical1[0](1)*projected_vertical1[0](1));
              double angle = acos(projected_vertical1[0](1)/direction_norm);
              if(projected_vertical1[0](0) > 0.0)
              {
                  angle = -angle;
              }
              std::cout << "The angle is:" << angle << std::endl;

              if(angle > 0.0)
              {
                  angle = M_PI - angle;
              }
              else
              {
                     angle = fabs(angle);
              }
              std::cout << "The transformed angle is:" << angle << std::endl;
              double angle_increment = M_PI / ((double) (9));
              std::cout << "The left bin is: " <<  std::floor(angle/angle_increment) << std::endl;*/
              double center_x = 10.0 + j *20.0;
              double center_y = 10.0 + i *20.0;
              get_line_segments_for_drawing(line_segments, projected_horizontal, center_x, center_y, length);
              int ooffset = i*(num_hog_cols*9) + j*9;
              //int input_offset = i*(num_hog_cols*32) + j*32;
              for(int k = 0; k < line_segments.size(); k++)
              {
                  double weight = (hog_responses_horizontal[ooffset + k]/hog_maxs(0))*255.0;
                  //weight = 255.0;
                  /*std::cout << "The weight is:" << (hog_responses_vertical_1[ooffset + k]) << std::endl;
                  std::cout << "The max is:" << hog_maxs(0) << std::endl;
                  std::cout << "The inerpolated ones are:" << (hog_responses_2D[input_offset])
                           << "   and " << (hog_responses_2D[input_offset + 8]) <<  std::endl;
                  int left_2D_vector = std::floor(angle/angle_increment);
                  int right_2D_vector = left_2D_vector + 1;
                  if(right_2D_vector >=(9))
                  {
                      right_2D_vector = 0;
                  }
                  std::cout << "The left - right bins are: " << left_2D_vector << "  - " << right_2D_vector << std::endl;
                  double ratio_1 = fmod(angle, angle_increment)/angle_increment;
                  std::cout << "The ratio is: " << ratio_1 << std::endl;
                  std::cout << "The validatede weight is:" << (hog_responses_2D[input_offset+left_2D_vector]*(1 - ratio_1)) + (hog_responses_2D[input_offset + right_2D_vector]*(ratio_1)) << std::endl;

                  //std::cout << "The weight is:" << weight << std::endl;*/
                      //line_segments[0].draw(img, weight, weight, weight, 0.2);
                  //if( (k == 0) || (k == 1) || (k == 2) || (k == 3) || (k == 4))
                  if(k == 0)
                  {
                      //line_segments[k].draw(img, weight, weight, weight, 0.2);
                	  line_segments[k].draw(img, 255.0, 255.0, 255.0, 0.2);
                  }
                  if(k == 8)
                  {
                      //line_segments[k].draw(img, weight, weight, weight, 0.2);
                	  line_segments[k].draw(img, 0.0, 0.0, 255.0, 0.2);
                  }
                  if(k == 7)
                  {
                      //line_segments[k].draw(img, weight, weight, weight, 0.2);
                	  line_segments[k].draw(img, 0.0, 255.0, 0.0, 0.2);
                  }
                  if(k == 1)
                  {
                      //line_segments[k].draw(img, weight, weight, weight, 0.2);
                	  line_segments[k].draw(img, 255.0, 0.0, 0.0, 0.2);
                  }
                  //line_segments[0].draw(img, 255.0, 255.0, 255.0, 0.2);
                      //line_segments[5].draw(img, 255.0, 255.0, 255.0, 0.2);
                  //break;
              }
              //break;
          }
          //break;
    }

    return img;
}

void Manhattan_hog::get_hog_pixels(std::vector<std::pair<int, int> > & hog_pixels) const
{
    hog_pixels.clear();
    for(int i = 0; i < num_hog_rows; i++)
    {
    	for(int j = 0; j < num_hog_cols; j++)
    	{
    		hog_pixels.push_back(std::pair<int, int>(get_hog_cell_y_center(i), get_hog_cell_x_center(j)));
    	}
    }
}

void Manhattan_hog::get_hog_offsets(std::vector<int>& hog_offsets) const
{
	hog_offsets.clear();
    for(int i = 0; i < num_hog_rows; i++)
    {
    	for(int j = 0; j < num_hog_cols; j++)
    	{
    		hog_offsets.push_back(i*(num_hog_cols*9) + j*9);
    	}
    }
}
