/* $Id$ */

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
|     Luca Del Pero
|
* =========================================================================== */

#include <st_cpp/st_parapiped.h>
#include <st_cpp/st_perspective_camera.h>
#include <edge_cpp/manhattan_world.h>
#include <edge_cpp/features_manager.h>
#include <gr_cpp/gr_offscreen.h>
#include <iostream>

#warning "[Code police] doesn't compile as of 12 Oct. 2012; see below."


using namespace std;
using namespace kjb;



//Temporary, fix it so that it works on Linux as well
int main(int argc, char **argv)
{
    if(argc < 3)
    {
        std::cout << "Usage: <base_dir> <input_image>" << std::endl;
                return 0;
    }

    string base(argv[1]);
    base.append("/");

    string features_file(base);
    features_file.append("validation/features/");
    features_file.append(argv[2]);
    features_file.append("_features.txt");

    string features_file2(base);
    features_file2.append("/features/");
    features_file2.append(argv[2]);
    features_file2.append("_features.txt");

    string image_name(base);
    image_name.append(argv[2]);
    image_name.append(".jpg");
    Image img(image_name);

    string out_name(base);
    out_name.append("validation/block_in_block_proposal/");
    out_name.append(argv[2]);
    out_name.append(".jpg");

    base.append("gt/right/");
    base.append(argv[2]);
    string camera_name(base);
    camera_name.append("_camera.txt");

    string pp_name(base);
    pp_name.append("_room.txt");

    Perspective_camera camera(camera_name.c_str());
    Parametric_parapiped pp(pp_name.c_str());
    pp.update_if_needed();
    bool found = false;

    Vector desired_dimensions(3,0.0);
    desired_dimensions(0) = 0.3;
    desired_dimensions(1) = 0.3;
    desired_dimensions(2) = 0.3;


    Features_manager fm(features_file.c_str());
    Features_manager fm2(features_file2.c_str());
    unsigned int corner_index = 0;
    for(unsigned int i = 0; i < fm.get_manhattan_world().get_num_corners3(); i++)
    {
        try
        {
            const Manhattan_corner & corner = fm.get_manhattan_world().get_corner_3(i);
            /*kjb::robustly_propose_parapiped_and_camera_from_orthogonal_corner(pp, camera, corner,
                fm.get_manhattan_world().get_focal_length(), img.get_num_rows(), img.get_num_cols(), 1000);*/
            found = true;
            corner_index = i;
            break;
        }
        catch(KJB_error e)
        {
            continue;
        }
    }

    if(!found)
    {
        std::cout << "Could not do it!" << std::endl;
        KJB_THROW_2(KJB_error,"Could not do it");
    }

    static kjb::Offscreen_buffer* offscreen = 0;
    offscreen = kjb::create_and_initialize_offscreen_buffer(img.get_num_cols(), img.get_num_rows());

    camera.prepare_for_rendering(true);


    /************************************/

    double princ_x = img.get_num_cols()/2.0;
    double princ_y = img.get_num_rows()/2.0;
    corner_index = 0;
    const Manhattan_corner & corner = fm2.get_manhattan_world().get_corner_2(corner_index); //42
    Vector corner_position(4,1.0);
    corner_position(0) = (corner.get_position() )(0) - princ_x;
    corner_position(1) = - ((corner.get_position() )(1) - princ_y);
    corner_position(2) = - camera.get_focal_length();

    Vector direction1;
    Vector direction2;
    Vector direction3;
    corner.get_direction(0, direction1);
    corner.get_direction(1, direction2);
    corner.get_direction(2, direction3);


    bool expand_up = true;
    if(direction3(1) > 0.0)
    {
        expand_up = false;
    }

    /*direction1 *= 5;
    direction2 *= 5;
    direction3 *= 5;*/
    Vector vertex1(4,1.0);
    Vector vertex2(4,1.0);
    Vector vertex3(4,1.0);

    vertex1(0) = (corner.get_position() )(0) + direction1(0);
    vertex2(0) = (corner.get_position() )(0) + direction2(0);
    vertex3(0) = (corner.get_position() )(0) + direction3(0);

    vertex1(1) =  (corner.get_position() )(1) + direction1(1);
    vertex2(1) =  (corner.get_position() )(1) + direction2(1);
    vertex3(1) =  (corner.get_position() )(1) + direction3(1);

    vertex1(2) = - camera.get_focal_length();
    vertex2(2) = - camera.get_focal_length();
    vertex3(2) = - camera.get_focal_length();

    vertex1(0) -= princ_x;
    vertex2(0) -= princ_x;
    vertex3(0) -= princ_x;
    vertex1(1) = - (vertex1(1) - princ_y);
    vertex2(1) = - (vertex2(1) - princ_y);
    vertex3(1) = - (vertex3(1) - princ_y);

    Vector vertex_position(4,1.0);
    vertex_position(0) = 0.0;
    vertex_position(1) = 0.0;
    vertex_position(2) = - camera.get_focal_length();

    Vector camera_centre = camera.get_camera_centre();
    std::cout << "In camera coords:" << corner_position << std::endl;
    camera.get_point_in_world_coordinates(vertex_position, vertex_position);
    camera.get_point_in_world_coordinates(corner_position, corner_position);
    camera.get_point_in_world_coordinates(camera_centre, camera_centre);
    camera.get_point_in_world_coordinates(vertex1, vertex1);
    camera.get_point_in_world_coordinates(vertex2, vertex2);
    camera.get_point_in_world_coordinates(vertex3, vertex3);

#warning "[Code police] method get_orthogonal_segment is no longer valid."
#ifndef PROGRAMMER_IS_predoehl
    corner.get_orthogonal_segment(0).draw(img,0,0,255);
    corner.get_orthogonal_segment(1).draw(img,0,0,255);
    corner.get_orthogonal_segment(2).draw(img,0,0,255);
#endif
    glColor3f(255.0, 0.0, 0.0);
    Base_gl_interface::set_gl_view(img);
    glBegin(GL_LINES);
    glVertex4d(corner_position(0), corner_position(1), corner_position(2), corner_position(3));
    glVertex4d(vertex1(0), vertex1(1), vertex1(2), vertex1(3));
    glVertex4d(corner_position(0), corner_position(1), corner_position(2), corner_position(3));
    glVertex4d(vertex2(0), vertex2(1), vertex2(2), vertex2(3));
    glVertex4d(corner_position(0), corner_position(1), corner_position(2), corner_position(3));
    glVertex4d(vertex3(0), vertex3(1), vertex3(2), vertex3(3));
    glEnd();
    kjb_c::KJB_image * capture4 = NULL;
    Base_gl_interface::capture_gl_view(&capture4);
    Image img2(capture4);

    Vector camera_centre_in_room;
    Vector corner_position_in_room;

    /*std::cout << "In world coordinates:" << std::endl;
    std::cout << "Camera:" << camera_centre(0) << " | " << camera_centre(1) << " | " << camera_centre(2) << std::endl;
    std::cout << "Room:" << corner_position(0) << " | " << corner_position(1) << " | " << corner_position(2) << std::endl;*/
    pp.get_point_in_parapiped_coordinates(camera_centre, camera_centre_in_room);
    pp.get_point_in_parapiped_coordinates(corner_position, corner_position_in_room);
    pp.get_point_in_parapiped_coordinates(vertex1, vertex1);
    pp.get_point_in_parapiped_coordinates(vertex2, vertex2);
    pp.get_point_in_parapiped_coordinates(vertex3, vertex3);


    //std::cout << "Corner position:" << corner_position_in_room << std::endl;

    Vector camera_to_corner(4,0.0);
    double norm = 0.0;
    for(unsigned int i = 0; i < 3; i ++)
    {
        camera_to_corner(i) = corner_position_in_room(i) - camera_centre_in_room(i);
        norm += camera_to_corner(i)*camera_to_corner(i);
        vertex1(i) -=  camera_centre_in_room(i);
        vertex2(i) -=  camera_centre_in_room(i);
        vertex3(i) -=  camera_centre_in_room(i);
    }

    vertex1(3) = 0.0;
    vertex2(3) = 0.0;
    vertex3(3) = 0.0;
    vertex1 = vertex1.normalize();
    vertex2 = vertex2.normalize();
    vertex3 = vertex3.normalize();

    norm = sqrt(norm);
    camera_to_corner /= norm;
    camera_to_corner(3) = 1.0;

    std::cout << "Camera to corner:" << camera_to_corner << std::endl;

    Vector x_plane(4, 0.0);
    x_plane(0) = 1.0;

    Vector y_plane(4, 0.0);
    y_plane(1) = 1.0;

    Vector z_plane(4, 0.0);
    z_plane(2) = 1.0;

    //bool up = true;
    double pp_height = pp.get_height();
    double pp_length = pp.get_length();
    double pp_width = pp.get_width();

    desired_dimensions(0) *= pp_width;

    desired_dimensions(1) *= pp_height;
    desired_dimensions(2) *= pp_length;
    if(camera_to_corner(1) >= 0.0)
    {
        if(expand_up)
        {
            std::cout << "ERROR, this should be a frame" << std::endl;
        }

        y_plane(3) = - pp_height/2.0;

    }
    else
    {
        //up = false;
        y_plane(3) = pp_height/2.0;
    }

    //bool right = true;
    if(camera_to_corner(0) >= 0.0)
    {
        x_plane(3) = -pp_width / 2.0;
    }
    else
    {
        x_plane(3) = pp_width/2.0;
        //right = false;
    }

    //bool z_up = true;
    if(camera_to_corner(2) >= 0.0 )
    {
        z_plane(3) = -pp_length/2.0;
    }
    else
    {
        z_plane(3) = pp_length/2.0;
        //z_up = false;
    }

    Vector intersection;
    double t_x = 0.0;
    double max_t;
    bool found_t = false;
    if(kjb::intersect_3D_line_with_plane(intersection, t_x, camera_centre_in_room, camera_to_corner, x_plane) )
    {
        assert(t_x);
        max_t = t_x;
        found_t = true;
    }
    double t_y = 0.0;
    if(kjb::intersect_3D_line_with_plane(intersection, t_y, camera_centre_in_room, camera_to_corner, y_plane) )
    {
        if(!found_t)
        {
            assert(t_y);
            max_t = t_y;
            found_t = true;
        }
        else if(t_y < max_t)
        {
            max_t = t_y;
        }
    }
    double t_z = 0.0;
    if(kjb::intersect_3D_line_with_plane(intersection, t_z, camera_centre_in_room, camera_to_corner, z_plane) )
    {
        if(!found_t)
        {
            assert(t_z);
            max_t = t_z;
            found_t = true;
        }
        else if(t_z < max_t)
        {
            max_t = t_z;
        }
    }

    if(!found_t)
    {
        KJB_THROW_2(KJB_error,"Could not find intersection of vector with room planes");
    }

    double chosen_t = 0.0;
    bool need_to_expand = false;
    double expand_x = 0.0;
    double expand_y = 0.0;
    double expand_z = 0.0;
    bool expand_room_right = true;
    bool expand_room_up = expand_up;
    bool expand_room_z_up = true;
    if( (t_y > max_t) && (expand_up))
    {
        std::cout << "We need to expand the room!" << std::endl;
        chosen_t = t_y;
        need_to_expand = true;

    }
    else if(expand_up)
    {
        std::cout << "We are here" << std::endl;
        std::cout << "Tx:" << t_x << std::endl;
        std::cout << "Ty:" << t_y << std::endl;
        std::cout << "Tz:" << t_z << std::endl;
        chosen_t = t_y;
    }
    else
    {
        double chosen_ratio = 0.5; //0.5
        chosen_t = chosen_ratio*max_t;
    }

    Vector corner_3D_in_room(4, 1.0);
    for(unsigned int i = 0; i < 3; i++)
    {
        corner_3D_in_room(i) = camera_centre_in_room(i) + chosen_t*camera_to_corner(i);
    }

    if(corner_3D_in_room(0) < 0.0)
    {
        expand_room_right = false;
    }
    if(corner_3D_in_room(2) < 0.0)
    {
        expand_room_z_up = false;
    }

    if(need_to_expand)
    {
        if( fabs(corner_3D_in_room(0)) > (pp_width/2.0) )
        {
            expand_x = fabs(corner_3D_in_room(0)) - (pp_width/2.0) ;
            if(!expand_room_right)
            {
                corner_3D_in_room(0) += (expand_x/2.0);
            }
            else
            {
                corner_3D_in_room(0) -= (expand_x/2.0);
            }
            pp_width += expand_x;
        }
        if( fabs(corner_3D_in_room(2)) > (pp_length/2.0) )
        {
            expand_z = fabs(corner_3D_in_room(2)) - (pp_length/2.0) ;
            if(!expand_room_z_up)
            {
                corner_3D_in_room(2) += (expand_z/2.0);
            }
            else
            {
                corner_3D_in_room(2) -= (expand_z/2.0);
            }
            pp_length += expand_z;
        }
    }
    double t1 = ( corner_3D_in_room(1) - camera_centre_in_room(1) ) /vertex1(1);

    /** The first edge expands mostly along the x axis,
     * the second along the z axis */
    double x1_edge = camera_centre_in_room(0) + t1*vertex1(0);
    double z1_edge = camera_centre_in_room(2) + t1*vertex1(2);

    double t2 = ( corner_3D_in_room(1) - camera_centre_in_room(1) ) /vertex2(1);

    double x2_edge = camera_centre_in_room(0) + t2*vertex2(0);
    double z2_edge = camera_centre_in_room(2) + t2*vertex2(2);


    x1_edge -= corner_3D_in_room(0);
    x2_edge -= corner_3D_in_room(0);

    z1_edge -= corner_3D_in_room(2);
    z2_edge -= corner_3D_in_room(2);

    /** The first edge expands mostly along the x axis,
         * the second along the z axis */
    if(fabs(x2_edge) > fabs(x1_edge))
    {
        double temp = x1_edge;
        x1_edge = x2_edge;
        x2_edge = temp;
        temp = z1_edge;
        z1_edge = z2_edge;
        z2_edge = temp;
    }

    if(camera_to_corner(0) > 0.0)
    {
        if(camera_to_corner(2) > 0.0)
        {
            /** ^
             *  | ->
             */
            if( (x1_edge < 0) && (z2_edge < 0))
            {
                x1_edge = - x1_edge;
                z2_edge = - z2_edge;
                //std:: cout << "SWAP 1" << std::endl;
            }
        }
        else
        {
            /**
             *  | ->
             *  v
             */
            if( (x1_edge < 0) && (z2_edge > 0))
            {
                x1_edge = - x1_edge;
                z2_edge = - z2_edge;
                //std:: cout << "SWAP 2" << std::endl;
            }
        }
    }
    else
    {
        if(camera_to_corner(2) > 0.0)
        {
            /**    ^
             *   <-|
             *
             */
            if( (x1_edge > 0) && (z2_edge < 0))
            {
                x1_edge = - x1_edge;
                z2_edge = - z2_edge;
                //std:: cout << "SWAP 3" << std::endl;
            }
        }
        else
        {
           /**
             *  <-|
             *    v
            */
            if( (x1_edge > 0) && (z2_edge > 0))
            {
                x1_edge = - x1_edge;
                z2_edge = - z2_edge;
                //std:: cout << "SWAP 4" << std::endl;
            }

        }
    }

    std::cout << "Edge 1:" << x1_edge << " | " <<  z1_edge << std::endl;
    std::cout << "Edge 2:" << x2_edge << " | " <<  z2_edge << std::endl;

    bool expand_right = true;
    if(x1_edge < 0.0)
    {
        expand_right = false;
    }
    bool expand_z_up = true;
    if(z2_edge < 0.0)
    {
        expand_z_up = false;
    }

    /** Now find the max size allowed */
    double max_size_x = 0.0;
    if(expand_right)
    {
        max_size_x = ( pp_width/2.0 ) - corner_3D_in_room(0);
        std::cout << "Max size x:" << max_size_x << std::endl;
    }
    else
    {
        max_size_x = fabs( ( -pp_width/2.0 ) - corner_3D_in_room(0) );
        std::cout << "Max size z:" << max_size_x << std::endl;
    }
    double max_size_y = 0.0;
    if(expand_up)
    {
        std::cout << "Expand UP" << std::endl;
        std::cout << corner_3D_in_room(1) << std::endl;
        max_size_y = ( pp_height/2.0 ) - corner_3D_in_room(1);
    }
    else
    {
        std::cout << "Expand DOWN" << std::endl;
        max_size_y = fabs( ( -pp_height/2.0 ) - corner_3D_in_room(1) );
    }
    double max_size_z = 0.0;
    if(expand_z_up)
    {
        max_size_z = ( pp_length/2.0 ) - corner_3D_in_room(2);
    }
    else
    {
        max_size_z = fabs( ( -pp_length/2.0 ) - corner_3D_in_room(2) );
    }

    if(max_size_x < desired_dimensions(0))
    {
        double temp_expand_x = desired_dimensions(0) - max_size_x;
        pp_width += temp_expand_x;
        max_size_x = desired_dimensions(0);
        if(!expand_right)
        {
            corner_3D_in_room(0) += (temp_expand_x/2.0);
        }
        else
        {
            corner_3D_in_room(0) -= (temp_expand_x/2.0);
        }

        if(expand_right == expand_room_right)
        {
            expand_x += temp_expand_x;
        }
        else
        {
            expand_x -= temp_expand_x;
            if(expand_x < 0.0)
            {
                expand_x = -expand_x;
                expand_room_right = !expand_room_right;
            }
        }
        need_to_expand = true;
    }
    if(max_size_y < desired_dimensions(1))
    {
        double temp_expand_y = desired_dimensions(1) - max_size_y;
        max_size_y = desired_dimensions(1);
        pp_height += temp_expand_y;
        if(!expand_up)
        {
            corner_3D_in_room(1) += (temp_expand_y/2.0);
        }
        else
        {
            corner_3D_in_room(1) -= (temp_expand_y/2.0);
        }

        if(expand_up == expand_room_up)
        {
            expand_y += temp_expand_y;
        }
        else
        {
            expand_y -= temp_expand_y;
            if(expand_y < 0.0)
            {
                expand_y = -expand_y;
                expand_room_up = !expand_room_up;
            }
        }
        need_to_expand = true;
    }
    if(max_size_z < desired_dimensions(2))
    {
        double temp_expand_z = desired_dimensions(2) - max_size_z;
        pp_length += temp_expand_z;
        max_size_z = desired_dimensions(2);
        if(!expand_z_up)
        {
            corner_3D_in_room(2) += (temp_expand_z/2.0);
        }
        else
        {
            corner_3D_in_room(2) -= (temp_expand_z/2.0);
        }
        if(expand_z_up == expand_room_z_up)
        {
            expand_z += temp_expand_z;
        }
        else
        {
            expand_z -= temp_expand_z;
            if(expand_z < 0.0)
            {
                expand_z = -expand_z;
                expand_room_z_up = !expand_room_z_up;
            }
        }
        need_to_expand = true;
    }
    /*max_size_x = 100;
    max_size_z = 100;*/

    std::cout << "Need to expand:" << need_to_expand << std::endl;
    std::cout << "Expand x:" << expand_x << std::endl;
    std::cout << "Expand y:" << expand_y << std::endl;
    std::cout << "Expand z:" << expand_z << std::endl;
    std::cout << "Corner 3D:" << corner_3D_in_room << std::endl;
    std::cout << "Max size x:" << max_size_x << std::endl;
    std::cout << "Max size y:" << max_size_y << std::endl;
    std::cout << "Max size z:" << max_size_z << std::endl;
    std::cout << "Expand x:" << expand_right << std::endl;
    std::cout << "Expand y:" << expand_up << std::endl;
    std::cout << "Expand z up:" << expand_z_up << std::endl;

    /** For now we keep the max size */
    Vector inner_centre(4, 1.0);
    inner_centre(0) = corner_3D_in_room(0);
    inner_centre(1) = corner_3D_in_room(1);
    inner_centre(2) = corner_3D_in_room(2);

    double width = max_size_x;
    double height = max_size_y;
    double length = max_size_z;
    /*double width = desired_dimensions(0);
    double height = desired_dimensions(1);
    double length = desired_dimensions(2);*/
    if(expand_right)
    {
        inner_centre(0) +=  width/2.0;
    }
    else
    {
        inner_centre(0) -= width/2.0;
    }
    if(expand_up)
    {
        inner_centre(1) += height/2.0;
    }
    else
    {
        height = fabs( corner_3D_in_room(1) + (pp_height/2.0 ));
        inner_centre(1) -= height/2.0;
    }
    if(expand_z_up)
    {
        inner_centre(2) += length/2.0;
    }
    else
    {
        inner_centre(2) -= length/2.0;
    }

    std::cout << "Inner centre:" << inner_centre << std::endl;

    if(need_to_expand)
    {
        if(expand_x > DBL_EPSILON)
        {
            std::cout << "Expand x:" << expand_x << std::endl;
            //pp.stretch_along_axis(0, expand_x, expand_room_right);
            pp.stretch_along_axis(0, expand_x, expand_room_right);
        }
        if(expand_y > DBL_EPSILON)
        {
            std::cout << "Expand y" << expand_y << std::endl;
            pp.stretch_along_axis(1, expand_y, expand_room_up);
        }
        if(expand_z > DBL_EPSILON)
        {
            std::cout << "Expand z" << expand_z << std::endl;
            pp.stretch_along_axis(2, expand_z, expand_room_z_up);
        }
    }
    pp.get_point_in_world_coordinates(inner_centre, inner_centre);

    std::cout << "Centre:" << inner_centre(0) << " | " << inner_centre(1) << " | " << inner_centre(2) << std::endl;
    std::cout << "Dimensions:" << width << " | " << height << " | " << length << std::endl;
    //Parametric_parapiped inner_pp(inner_centre(0), inner_centre(1), inner_centre(2), width, height, length, 0.0,  pp.get_yaw(), 0.0);
    Parametric_parapiped inner_pp(inner_centre(0), inner_centre(1), inner_centre(2), width, height, length, 0.0,  pp.get_yaw(), 0.0);
    glColor3f(255.0, 0.0, 0.0);
    camera.prepare_for_rendering(true);
    Base_gl_interface::set_gl_view(img);

    pp.wire_render();
    glColor3f(0.0, 0.0, 255.0);
    inner_pp.wire_render();
    kjb_c::KJB_image * capture5 = NULL;
    Base_gl_interface::capture_gl_view(&capture5);
    Image img3(capture5);
    img3.write("inner.jpg");

    /*std::cout << "Edge 1: " << x1_edge << " | " << corner_3D_in_room(1) << " | " << z1_edge << std::endl;
    std::cout << "Edge 2: " << x2_edge << " | " << corner_3D_in_room(1) << " | " << z2_edge << std::endl;
    std::cout << "Corner: " << corner_3D_in_room(0)  << " | " << corner_3D_in_room(1) << " | " << corner_3D_in_room(2)  << std::endl;
    std::cout << "Camera: :" << camera_centre_in_room(0) << " | " << camera_centre_in_room(1) << " | " << camera_centre_in_room(2) << std::endl;
    std::cout << "Room dimensions:"<< pp.get_width() << " | " << pp.get_height() << " | " << pp.get_length() << std::endl;
    std::cout << "Camera to corner:" << camera_to_corner(0) << " | " << camera_to_corner(1) << " | " << camera_to_corner(2) << std::endl;*/

    /************************************/



    img2.write(out_name.c_str());

    std::cout << "Yaw:" << pp.get_yaw() << std::endl;
}


/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

