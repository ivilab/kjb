
/* $Id: i_color_histogram.cpp 21596 2017-07-30 23:33:36Z kobus $ */

#include "i_cpp/i_color_histogram.h"
#include "m_cpp/m_vector_stream_io.h"

#include <fstream>
#include <sstream>
#include <iostream>

using namespace kjb;

/**
 * Constructor. The color histogram is computed over the entire image
 *
 * @param num_bins the number of bins in the histogram.
 *        This value is used to discretize each of the channels (r,g,b)
 * @param img the image to compute the histogram over
 */
Color_histogram::Color_histogram(unsigned int inum_bins,const Image & img)
    : Readable(), Writeable(),
    m_histogram((int)(inum_bins*inum_bins*inum_bins), 0.0), num_bins(inum_bins), bin_size(255.0 / num_bins)
{
    compute_histogram(img);
}

/**
 * Constructor. The color histogram is computed over the specified rectangular portion of the image
 *
 * @param num_bins the number of bins in the histogram.
 *        This value is used to discretize each of the channels (r,g,b)
 * @param img the image to compute the histogram over
 * @param top_left_x The top left x coordinate of the rectangular portion of the image to
 *        compute the histogram over. The origin of the image system is the top left corner
 *        of the image, with x increasing left to right, y increasing top to bottom
 * @param top_left_y The top left y coordinate of the rectangle
 * @param bottom_right_x The bottom right x coordinate of the rectangle
 * @param bottom_right_y The bottom right y coordinate of the rectangle
 */
Color_histogram::Color_histogram
(
    unsigned int inum_bins,
    const Image & img,
    int top_left_x,
    int top_left_y,
    int bottom_right_x,
    int bottom_right_y

)
    : Readable(), Writeable(),
     m_histogram((int)(inum_bins*inum_bins*inum_bins), 0.0), num_bins(inum_bins), bin_size(255.0 / num_bins)
{
    compute_histogram(img, top_left_x, top_left_y, bottom_right_x, bottom_right_y);
}

/**
 * Constructor. This constructor only initializes the memory for the histogram
 *
 * @param num_bins the number of bins in the histogram.
 *        This value is used to discretize each of the channels (r,g,b)
 */
Color_histogram::Color_histogram(unsigned int inum_bins)
    : Readable(), Writeable(),
    m_histogram((int)(inum_bins*inum_bins*inum_bins), 0.0), num_bins(inum_bins), bin_size(255.0 / num_bins)
{

}

/**
 * Copy constructor
 *
 * @param src The Color_histogram to copy into this one
 */
Color_histogram::Color_histogram(const Color_histogram & src)
    : Readable(src), Writeable(src),
    m_histogram(src.m_histogram), num_bins(src.num_bins), bin_size(src.bin_size)
{

}

/**
 * Assignment operator
 *
 * @param src The Color_histogram to assign to this one
 */
Color_histogram & Color_histogram::operator =(const Color_histogram & src)
{
    m_histogram = src.m_histogram;
    num_bins = src.num_bins;
    bin_size = src.bin_size;
    return (*this);
}

void Color_histogram::read(std::istream& in)
{
    Vector_stream_io::read_vector(in, m_histogram);
    num_bins = (int) m_histogram.size()/3.0;
    if(num_bins <= 0)
    {
        KJB_THROW_2(IO_error, "Histogram vector read from file has zero size!");
    }
    bin_size = 255.0 / num_bins;
}

void Color_histogram::write(std::ostream& out) const
{
    Vector_stream_io::write_vector(out, m_histogram);
}

void Color_histogram::compute_histogram(const kjb::Image & img, unsigned int inum_bins)
{
     compute_histogram(img, inum_bins, 0.0, 0.0, img.get_num_cols() -1, img.get_num_rows()-1);
}

void Color_histogram::compute_histogram(const kjb::Image & img)
{
    compute_histogram(img, 0.0, 0.0, img.get_num_cols()-1, img.get_num_rows()-1);
}

void Color_histogram::compute_histogram
(
    const kjb::Image & img,
    unsigned int inum_bins,
    int top_left_x,
    int top_left_y,
    int bottom_right_x,
    int bottom_right_y

)
{
    num_bins = inum_bins;
    bin_size = 255.0 / num_bins;
    compute_histogram(img, top_left_x, top_left_y, bottom_right_x, bottom_right_y);
}

void Color_histogram::compute_histogram
(
    const kjb::Image & img,
    int top_left_x,
    int top_left_y,
    int bottom_right_x,
    int bottom_right_y

)
{
    if( (top_left_x < 0) )
    {
        KJB_THROW_2(Illegal_argument, "Top left x coordinate of image region is out of bounds");
    }
    if( bottom_right_x >= img.get_num_cols() )
    {
        KJB_THROW_2(Illegal_argument, "Bottom right x coordinate of image region is out of bounds");
    }
    if( bottom_right_x <= top_left_x)
    {
        KJB_THROW_2(Illegal_argument, "right x coordinate of image region is smaller then left x coordinate");
    }
    if( (top_left_y < 0) )
    {
        KJB_THROW_2(Illegal_argument, "Top left y coordinate of image region is out of bounds");
    }
    if( bottom_right_y >= img.get_num_rows() )
    {
        KJB_THROW_2(Illegal_argument, "Bottom right y coordinate of image region is out of bounds");
    }
    if( bottom_right_y <= top_left_y)
    {
        KJB_THROW_2(Illegal_argument, "bottom y coordinate of image region is smaller then top y coordinate");
    }

    m_histogram.zero_out((int)(num_bins*num_bins*num_bins));

    for(int i = top_left_x; i < bottom_right_x; i++)
    {
        for(int j = top_left_y; j < bottom_right_y; j++)
        {
            m_histogram[ compute_rgb_bin(img(j,i,0), img(j,i,1), img(j,i,2)) ]++;
        }
    }

    int area =  ((bottom_right_x - top_left_x)*(bottom_right_y - top_left_y));
    std::cout << "area: " << area << std::endl;
    m_histogram /= ((bottom_right_x - top_left_x)*(bottom_right_y - top_left_y));
}

unsigned int Color_histogram::compute_rgb_bin(float r, float g, float b) const
{
    unsigned int r_bin = find_bin(r);
    unsigned int g_bin = find_bin(g);
    unsigned int b_bin = find_bin(b);
    return ( (num_bins * (r_bin*num_bins + g_bin) ) + b_bin );
}

unsigned int Color_histogram::find_bin(float ivalue) const
{
    if(ivalue < 0.0 || (ivalue > 255.0) )
    {
        KJB_THROW_2(Illegal_argument, "RGB value should be between 0 and 255");
    }
    unsigned int bin = round( (ivalue/bin_size) );
    if(bin == (num_bins))
    {
        bin--;
    }
    if(bin > num_bins)
    {
        KJB_THROW_2(KJB_error, "Bin index out of bounds");
    }
    return bin;
}


void Color_histogram::print_human_readable(std::ostream & out) const
{
    for(unsigned int r = 0; r < num_bins; r++)
    {
        int start_r_bin = (int) r*bin_size;
        int end_r_bin = start_r_bin + bin_size;
        for(unsigned g = 0; g < num_bins; g++)
        {
            int start_g_bin = (int) g*bin_size;
            int end_g_bin = start_g_bin + bin_size;
            for(unsigned int b = 0; b < num_bins; b++)
            {
                int start_b_bin = (int) b*bin_size;
                int end_b_bin = start_b_bin + bin_size;
                int temp_bin = (num_bins * (r*num_bins + g) ) + b;
                out << "R:[" << start_r_bin << "-" << end_r_bin << "],"
                        << "G:[" << start_g_bin << "-" << end_g_bin << "],"
                        << " B:[" << start_b_bin << "-" << end_b_bin << "] = " << m_histogram[temp_bin] << std::endl;
            }
        }
    }
}


double Color_histogram::compare(const Color_histogram & ch) const
{
    if(ch.get_num_bins() != num_bins)
    {
        KJB_THROW_2(KJB_error, "Trying to compare histograms with different number of bins!!!");
    }
    double diff = 0.0;
    for(int i = 0; i < m_histogram.size(); i++)
    {
        diff += (m_histogram[i] - ch.get_value(i))*(m_histogram[i] - ch.get_value(i));
    }
    return sqrt(diff);
}

void kjb::write_histograms(const std::vector<kjb::Color_histogram> & chs, const std::string & file_name)
{
    std::ofstream out(file_name.c_str());
    if(out.fail())
    {
        KJB_THROW_2(IO_error, "Could not open file for writing histograms: ");
    }
    out << "     Quantity: " << chs.size() << std::endl;
    for(unsigned int i = 0; i < chs.size(); i++)
    {
        chs[i].write(out);
    }

    out.close();
}

void kjb::read_histograms(std::vector<kjb::Color_histogram> & chs, const std::string & file_name)
{
    chs.clear();
    std::ifstream in(file_name.c_str());
    if(in.fail())
    {
        KJB_THROW_2(IO_error, "Could not open file for reading histograms: ");
    }

    const char* field_value;
    // Quantity
    if (!(field_value = kjb::Readable::read_field_value(in, "Quantity")))
    {
        KJB_THROW_2(Illegal_argument, "Missing Type Quantity");
    }
    std::istringstream ist(field_value);
    int quantity = 0;
    ist >> quantity;
    if (ist.fail())
    {
        KJB_THROW_2(Illegal_argument, "Invalid Number of histograms");
    }
    for(int i = 0; i < quantity; i++)
    {
        chs.push_back(Color_histogram(in));
    }
    in.close();
}
