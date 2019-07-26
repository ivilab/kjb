// throwaway code

#include <l_cpp/l_cpp_incl.h>
#include <m_cpp/m_cpp_incl.h>

#include <gr_cpp/gr_2D_bounding_box.h>

#include <iostream>
#include <sstream>
#include <string>

#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>

#include <i/i_float.h>

#include <psi_cpp/psi_human_box.h>

using namespace kjb::psi;

void draw_boxes(kjb::Image& img, const std::vector<kjb::Bounding_Box2D>& boxes)
{
    for(size_t i = 0; i < boxes.size(); i++)
    {
    	if(i == 0)
    	{
    		boxes[i].draw(img, 255.0, 0.0, 0.0);
    	}
    	else
    	{
    	    boxes[i].draw(img, 0., 0., 255.);
    	}
    }
}

void print_usage(const char* cmd_name)
{
    std::cout << "Usage: " << cmd_name << " path_to_frames path_to_boxes path_to_output_dir" << std::endl;
}

int main(int argc, char* argv[])
{
    using std::string;
    using kjb::Image;
    using std::vector;
    // image filename, boxes filename
    // output image with boxes
    //
    if(argc != 5)
    {
        print_usage(argv[0]);
        exit(1);
    }

    string img_fname(argv[1]);
    img_fname.append("/");
    string out_fname(argv[3]);


    std::string frame_fmt = argv[2];
    boost::format fmt(frame_fmt);
    std::cout << "Frame fmt: " << frame_fmt << std::endl;
    int num_frames = atoi(argv[4]);
    std::cout << "Num frames: " << num_frames << std::endl;
    
    double best_score = -100.0;
    int best_frame = 0;
    kjb::psi::Human_boxes best_box;

    int j_index = 0;



    for(unsigned int i = 0; i < num_frames; i++)
    {
    	std::string frame_name = str(fmt % (i + 1));
        std::cout << frame_name << std::endl;
    	std::ifstream ifs(frame_name.c_str());
    	std::vector<kjb::psi::Human_boxes> hbs = parse_human_boxes(ifs);
    	for(unsigned int j = 0; j < hbs.size(); j++)
    	{
    		if(hbs[j].get_score() > best_score)
    		{
    			best_frame = i;
    			best_score = hbs[j].get_score();
    			best_box = hbs[j];
    			j_index = j;
    		}
    	}
    }

    double best_score_2 = -100.0;
    int best_frame2 = 0;
    kjb::psi::Human_boxes best_box2;

    for(unsigned int i = 0; i < num_frames; i++)
    {
	    std::string frame_name = str(fmt % (i + 1));
	    std::cout << frame_name << std::endl;
	    std::ifstream ifs(frame_name.c_str());
	    std::vector<kjb::psi::Human_boxes> hbs = parse_human_boxes(ifs);
	    for(unsigned int j = 0; j < hbs.size(); j++)
	    {
		   if((i == best_frame) && (j == j_index) )
		   {
			   continue;
		   }
		   double x_dist = (hbs[j].full_body().get_centre_x() - best_box.full_body().get_centre_x());
		   double y_dist = (hbs[j].full_body().get_centre_y() - best_box.full_body().get_centre_y());
		   double distance = sqrt((x_dist*x_dist)+(y_dist*y_dist));
		   if(distance < 100.0)
		   {
			   continue;
		   }
	       if(hbs[j].get_score() > best_score_2)
		   {
	    	    best_frame2 = i;
			    best_score_2 = hbs[j].get_score();
			    best_box2 = hbs[j];
		   }
	}
  }

    char frame_number_string[50];
    sprintf(frame_number_string, "%d", best_frame);
    string img_fname2(img_fname);
    string img_try(img_fname);
    img_fname.append("00");
    img_fname.append(frame_number_string);
    img_fname.append(".jpg");
    out_fname.append("/");
    string out_fname2(out_fname);

    Image img_in;
    try{
    	Image imgtemp(img_fname);
		img_in = imgtemp;
		out_fname.append("1_00");
		out_fname.append(frame_number_string);
		out_fname.append(".jpg");
		out_fname2.append("2_00");
		char frame_number_string2[50];
		sprintf(frame_number_string2, "%d", best_frame2);
		out_fname2.append(frame_number_string2);
		out_fname2.append(".jpg");
		string img_try2(img_fname2);
		img_fname2.append("00");
		img_fname2.append(frame_number_string2);
		img_fname2.append(".jpg");
    } catch(kjb::KJB_error e)
    {
    	img_fname = img_try;
    	img_fname.append("0");
		img_fname.append(frame_number_string);
		img_fname.append(".jpg");
		Image imgtemp(img_fname);
		img_in = imgtemp;
		char frame_number_string2[50];
		sprintf(frame_number_string2, "%d", best_frame2);
		out_fname.append("1_0");
		out_fname.append(frame_number_string);
		out_fname.append(".jpg");
		out_fname2.append("2_0");
		out_fname2.append(frame_number_string2);
		out_fname2.append(".jpg");
		string img_try2(img_fname2);
		img_fname2.append("00");
		img_fname2.append(frame_number_string2);
		img_fname2.append(".jpg");
    }

    Image img(img_fname);
    draw_boxes(img, best_box);
    Image imgb(img_fname2);
    draw_boxes(imgb, best_box2);

    img.write(out_fname);
    imgb.write(out_fname2);

    return 0;
}


