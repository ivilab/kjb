// throwaway code

#include <l_cpp/l_cpp_incl.h>
#include <m_cpp/m_cpp_incl.h>

#include <gr_cpp/gr_2D_bounding_box.h>

#include <iostream>
#include <sstream>
#include <string>

#include <i/i_float.h>

#include <psi_cpp/psi_human_box.h>

using namespace kjb::psi;

void print_usage(const char* cmd_name)
{
    std::cout << "Usage: " << cmd_name << " path_to_video_directory frame_number output_dir" << std::endl;
}

int main(int argc, char* argv[])
{
    using std::string;
    using kjb::Image;
    using std::vector;
    // image filename, boxes filename
    // output image with boxes
    //
    if(argc != 4)
    {
        print_usage(argv[0]);
        exit(1);
    }

    string frame_fname(argv[1]);
    frame_fname.append("/person/");
    char frame_str[50];
    sprintf(frame_str,"%05d", atoi(argv[2]));
    frame_fname.append(frame_str);
    frame_fname.append(".txt");
	std::ifstream ifs(frame_fname.c_str());
	std::vector<kjb::psi::Human_boxes> hbs = parse_human_boxes(ifs);

	string image_name(argv[1]);
	image_name.append("/frames/");
	image_name.append(frame_str);
	image_name.append(".jpg");
	string output_base(argv[3]);
	output_base.append("/");

	int count = 0;
	for(unsigned int i = 0; i < hbs.size(); i++ )
	{
		if(hbs[i].get_score() < -1.0)
		{
			continue;
		}

		char index_str[10];
		sprintf(index_str, "%d", count);
		string outname(output_base);
		outname.append(index_str);
		outname.append(".jpg");
		Image img(image_name);
		(hbs[i])[0].draw(img, 255.0, 0.0, 0.0);
		count++;
		img.write(outname);
	}

    return 0;
}


