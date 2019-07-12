#include "blob_cpp/blob_gss.h"
#include "i_cpp/i_filter.h"
#include <boost/lexical_cast.hpp>

void GSS::write(const std::string& base_filename, const std::string& extension)
{
    for(std::vector<Octave::const_iterator>::const_iterator p = x_octaves.begin(); p != x_octaves.end(); p++)
    {
        int o = p - x_octaves.begin() + o_min;
        for(Octave::const_iterator q = *p; q != *p + (s_max - s_min) + 1; q++)
        {
            int s = q - *p + s_min;
            std::string filename = base_filename + "_octave_";
            if(o < 0)
            {
                filename += "n";
            }
            filename += boost::lexical_cast<std::string>(std::abs(o));
            filename += "_level_";
            if(s < 0)
            {
                filename += "n";
            }
            filename += boost::lexical_cast<std::string>(std::abs(s));
            q->write(filename + "." + extension);
        }
    }
}

GSS GSS_generator::operator()(const kjb::Image& image)
{
    GSS gss(O, o_min, S, s_min, s_max, sigma_0, sigma_n);
    std::vector<kjb::Image>& gss_vector = gss.gss;
    std::vector<GSS::Octave::const_iterator>& octaves = gss.octaves;
    std::vector<GSS::Octave::const_iterator>& x_octaves = gss.x_octaves;

    double sigma;
    // sigma scale factor -- computed only once for efficiency
    double sigma_sf = sigma_0 * sqrt(1 - std::pow(2.0, -2.0 / S));

    // reserve memory for the vectors in advance
    gss_vector.reserve(1 + (s_max - s_min + 1) * O);
    octaves.reserve(O + 1);
    x_octaves.reserve(O + 1);

    // initial scaling of the image in case o_min =/= 0
    kjb::Image I = kjb::scale_image(image, std::pow(2.0, -o_min));

    //-----------------------------------------------------------------------------
    // First octave ---------------------------------------------------------------
    //
    // The first octave is computed here. First the first level is computed and
    // then the rest -- which use the first one. There is a little math involved
    // in computing the appropriate scale of the gaussian filters. If you need to
    // know, ask me (or figure it out yourself =D).

    //std::cout << "Octave: "<< o_min << "\n"; 

    // First level
    sigma = sqrt(std::pow(sigma_0 * std::pow(2.0, static_cast<double>(s_min) / S), 2) - std::pow(sigma_n / std::pow(2.0, o_min), 2));
    gss_vector.push_back(I * kjb::gaussian_filter(sigma));
    //std::cout << "  level " << s_min << " : " << sigma << std::endl; 

    // Other levels
    for(int s = s_min + 1; s <= s_max; s++)
    {
        sigma = std::pow(2.0, static_cast<double>(s) / S) * sigma_sf;
        gss_vector.push_back(gss_vector.back() * kjb::gaussian_filter(sigma));
        //std::cout<<"  level " << s << ": "<< sigma << " w ( " << gss_vector.back().get_num_rows() << " )" << std::endl; 
    }

    x_octaves.push_back(gss_vector.begin());
    octaves.push_back(gss_vector.begin() - s_min);
    //-----------------------------------------------------------------------------


    //-----------------------------------------------------------------------------
    // Other octaves --------------------------------------------------------------
    //
    // The rest of the octaves are computed. Note that they each is computed
    // using the previous one. Also notice the scaling of the image at each octave;
    // this is done to speed up the computation. As before, the first level is
    // computed separetely; it has to be treated this way because its 'previous'
    // image is in the previous octave.

    for(int o = 1; o < O; o++)
    {
        //std::cout << "Octave: " << o_min + o << std::endl; 

        // First level

        if(s_min + S <= s_max)
        {
            // May be able to use the previous image as-is (after scaling).
            GSS::Octave::const_iterator p_prev = x_octaves[o - 1] + S;
            gss_vector.push_back(scale_image(*p_prev, 0.5));
           // std::cout <<" first level sigma: 0.5, w (" << p_prev->get_num_rows()*0.5 << ") \n"; 
        }
        else
        {
            // Need to smooth previous image in space
            GSS::Octave::const_iterator p_prev = octaves[o - 1] + s_max;
            double next_sigma = sigma_0 * std::pow(2.0, static_cast<double>(s_min) / S);
            double prev_sigma = sigma_0 * std::pow(2.0, static_cast<double>(s_max - S) / S);
            sigma = sqrt(next_sigma * next_sigma - prev_sigma * prev_sigma);
            gss_vector.push_back(scale_image(*p_prev, 0.5) * kjb::gaussian_filter(sigma));
    //        std::cout << " first level sigma: "<< sigma << " w (" << p_prev->get_num_rows()*0.5 << ") " <<  std::endl; 
        }

        // Other levels
        for(int s = s_min + 1; s <= s_max; s++)
        {
            sigma = std::pow(2.0, static_cast<double>(s) / S) * sigma_sf;
            gss_vector.push_back(gss_vector.back() * kjb::gaussian_filter(sigma));
            //std::cout << "level: " << s << " sigma: "<< sigma <<
            //    " w (" << gss_vector.back().get_num_rows() << ") " << std::endl; 
        }

        x_octaves.push_back(octaves[o - 1] + s_max + 1);
        octaves.push_back(x_octaves[o] - s_min);
    }
    //-----------------------------------------------------------------------------

    return gss;
}

