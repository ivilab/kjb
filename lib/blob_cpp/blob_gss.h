#ifndef GSS_INCLUDED_H_
#define GSS_INCLUDED_H_

#include "l_cpp/l_exception.h"
#include "i_cpp/i_image.h"
#include <vector>
#include <utility>
#include <cmath>
#include <string>

class Gaussian_scale_space
{
private:
    typedef std::vector<kjb::Image> Octave;

    std::vector<kjb::Image> gss;
    const int O;
    const int o_min;
    const int S;
    const int s_min;
    const int s_max;
    const double sigma_0;
    const double sigma_n;
    std::vector<Octave::const_iterator> octaves;
    std::vector<Octave::const_iterator> x_octaves;

    friend class Gaussian_scale_space_generator;

private:
    Gaussian_scale_space
    (
        int num_octaves,
        int min_octave,
        int num_levels,
        int min_level,
        int max_level,
        double initial_sigma,
        double nominal_sigma
    );

public:
    const std::vector<kjb::Image>& as_vector() const;

    std::pair<Octave::const_iterator, Octave::const_iterator> get_octave(int o) const;

    std::pair<Octave::const_iterator, Octave::const_iterator> get_x_octave(int o) const;

    std::pair<Octave::const_iterator, Octave::const_iterator> get_octave_at_index(int i_o) const;

    std::pair<Octave::const_iterator, Octave::const_iterator> get_x_octave_at_index(int i_o) const;

    int get_num_octaves() const;

    int get_min_octave() const;

    int get_num_levels() const;

    int get_min_level() const;

    int get_max_level() const;

    double get_initial_sigma() const;

    double get_nominal_sigma() const;

    int get_octave_name(int idx) const;

    int get_scale_name(int idx) const;

    double sigma_at(int o, int s) const;

    double sigma_at_indices(int i_o, int i_s) const;

    void write(const std::string& base_filename, const std::string& extension);

    ~Gaussian_scale_space();
};

typedef Gaussian_scale_space GSS;

//----------------------------------------------------------------------------------------
// INLINE MEMBER FUNCTIONS ARE DEFINED BELOW
//----------------------------------------------------------------------------------------

inline
GSS::Gaussian_scale_space
(
    int num_octaves,
    int min_octave,
    int num_levels,
    int min_level,
    int max_level,
    double initial_sigma,
    double nominal_sigma
) :
        O(num_octaves),
        o_min(min_octave),
        S(num_levels),
        s_min(min_level),
        s_max(max_level),
        sigma_0(initial_sigma),
        sigma_n(nominal_sigma)
{}

//----------------------------------------------------------------------------------------

inline
const std::vector<kjb::Image>& GSS::as_vector() const
{
    return gss;
}

//----------------------------------------------------------------------------------------

inline 
std::pair<GSS::Octave::const_iterator, GSS::Octave::const_iterator> GSS::get_octave(int o) const
{
    if(o - o_min < 0 || static_cast<size_t>(o - o_min) >= octaves.size())
    {
        KJB_THROW_2(kjb::Index_out_of_bounds, "That octave does not exist");
    }

    return std::make_pair(octaves[o - o_min], octaves[o - o_min] + S);
}

//----------------------------------------------------------------------------------------

inline
std::pair<GSS::Octave::const_iterator, GSS::Octave::const_iterator> GSS::get_x_octave(int o) const
{
    if(o - o_min < 0 || static_cast<size_t>(o - o_min) >= octaves.size())
    {
        KJB_THROW_2(kjb::Index_out_of_bounds, "That octave does not exist");
    }

    return std::make_pair(x_octaves[o - o_min], x_octaves[o - o_min] + (s_max - s_min) + 1);
}

//----------------------------------------------------------------------------------------

inline 
std::pair<GSS::Octave::const_iterator, GSS::Octave::const_iterator> GSS::get_octave_at_index(int i_o) const
{
    return get_octave(i_o + o_min);
}

//----------------------------------------------------------------------------------------

inline
std::pair<GSS::Octave::const_iterator, GSS::Octave::const_iterator> GSS::get_x_octave_at_index(int i_o) const
{
    return get_x_octave(i_o + o_min);
}

//----------------------------------------------------------------------------------------

inline
int GSS::get_num_octaves() const
{
    return O;
}

//----------------------------------------------------------------------------------------

inline
int GSS::get_min_octave() const
{
    return o_min;
}

//----------------------------------------------------------------------------------------

inline
int GSS::get_num_levels() const
{
    return S;
}

//----------------------------------------------------------------------------------------

inline
int GSS::get_min_level() const
{
    return s_min;
}

//----------------------------------------------------------------------------------------

inline
int GSS::get_max_level() const
{
    return s_max;
}

//----------------------------------------------------------------------------------------

inline
double GSS::get_initial_sigma() const
{
    return sigma_0;
}

//----------------------------------------------------------------------------------------

inline
double GSS::get_nominal_sigma() const
{
    return sigma_n;
}

//----------------------------------------------------------------------------------------

inline
int GSS::get_octave_name(int idx) const
{
    if(idx < 0 || idx >= O)
    {
        KJB_THROW_2(kjb::Index_out_of_bounds, "That octave index does not exist.");
    }

    return idx + o_min;
}

//----------------------------------------------------------------------------------------

inline
int GSS::get_scale_name(int idx) const
{
    if(idx < 0 || idx >= s_max - s_min + 1)
    {
        KJB_THROW_2(kjb::Index_out_of_bounds, "That scale index does not exist.");
    }

    return idx + s_min;
}

//----------------------------------------------------------------------------------------

inline
double GSS::sigma_at(int o, int s) const
{
    return sigma_0 * std::pow(2, o + static_cast<double>(s) / S);
}

//----------------------------------------------------------------------------------------

inline
double GSS::sigma_at_indices(int i_o, int i_s) const
{
    return sigma_at(get_octave_name(i_o), get_scale_name(i_s));
}

//----------------------------------------------------------------------------------------

inline
GSS::~Gaussian_scale_space()
{}


//----------------------------------------------------------------------------------------

class Gaussian_scale_space_generator
{
private:
    int O;
    int o_min;
    int S;
    int s_min;
    int s_max;
    double sigma_0;
    double sigma_n;

public:
    Gaussian_scale_space_generator(int num_octaves, int num_levels, double initial_sigma);

    Gaussian_scale_space_generator
    (
        int num_octaves,
        int min_octave,
        int num_levels,
        int min_level,
        int max_level,
        double initial_sigma,
        double nominal_sigma
    );

    ~Gaussian_scale_space_generator();

    GSS operator()(const kjb::Image& image);

    void set_num_octaves(int num_octaves);

    void set_min_octave(int min_octave);

    void set_num_levels(int num_levels);

    void set_min_level(int min_level);

    void set_max_level(int max_level);

    void set_initial_sigma(double initial_sigma);

    void set_nominal_sigma(double nominal_sigma);

    int get_num_octaves() const;

    int get_min_octave() const;

    int get_num_levels() const;

    int get_min_level() const;

    int get_max_level() const;

    double get_initial_sigma() const;

    double get_nominal_sigma() const;

private:
    void check_params() const;
};

typedef Gaussian_scale_space_generator GSS_generator;

//----------------------------------------------------------------------------------------
// INLINE MEMBER FUNCTIONS ARE DEFINED BELOW
//----------------------------------------------------------------------------------------

inline
GSS_generator::Gaussian_scale_space_generator
(
    int num_octaves,
    int num_levels,
    double initial_sigma
) :
        O(num_octaves),
        o_min(0),
        S(num_levels),
        s_min(0),
        s_max(num_levels - 1),
        sigma_0(initial_sigma),
        sigma_n(0.0)
{
    check_params();
}

//----------------------------------------------------------------------------------------

inline
GSS_generator::Gaussian_scale_space_generator
(
    int num_octaves,
    int min_octave,
    int num_levels,
    int min_level,
    int max_level,
    double initial_sigma,
    double nominal_sigma
) :
        O(num_octaves),
        o_min(min_octave),
        S(num_levels),
        s_min(min_level),
        s_max(max_level),
        sigma_0(initial_sigma),
        sigma_n(nominal_sigma)
{
    check_params();
}

//----------------------------------------------------------------------------------------

inline
GSS_generator::~Gaussian_scale_space_generator()
{}

//----------------------------------------------------------------------------------------

inline
void GSS_generator::set_num_octaves(int num_octaves)
{
    O = num_octaves;
    check_params();
}

//----------------------------------------------------------------------------------------

inline
void GSS_generator::set_min_octave(int min_octave)
{
    o_min = min_octave;
    check_params();
}

//----------------------------------------------------------------------------------------

inline
void GSS_generator::set_num_levels(int num_levels)
{
    S = num_levels;
    check_params();
}

//----------------------------------------------------------------------------------------

inline
void GSS_generator::set_min_level(int min_level)
{
    s_min = min_level;
    check_params();
}

//----------------------------------------------------------------------------------------

inline
void GSS_generator::set_max_level(int max_level)
{
    s_max = max_level;
    check_params();
}

//----------------------------------------------------------------------------------------

inline
void GSS_generator::set_initial_sigma(double initial_sigma)
{
    sigma_0 = initial_sigma;
    check_params();
}

//----------------------------------------------------------------------------------------

inline
void GSS_generator::set_nominal_sigma(double nominal_sigma)
{
    sigma_n = nominal_sigma;
    check_params();
}

//----------------------------------------------------------------------------------------

inline
int GSS_generator::get_num_octaves() const
{
    return O;
}

//----------------------------------------------------------------------------------------

inline
int GSS_generator::get_min_octave() const
{
    return o_min;
}

//----------------------------------------------------------------------------------------

inline
int GSS_generator::get_num_levels() const
{
    return S;
}

//----------------------------------------------------------------------------------------

inline
int GSS_generator::get_min_level() const
{
    return s_min;
}

//----------------------------------------------------------------------------------------

inline
int GSS_generator::get_max_level() const
{
    return s_max;
}

//----------------------------------------------------------------------------------------

inline
double GSS_generator::get_initial_sigma() const
{
    return sigma_0;
}

//----------------------------------------------------------------------------------------

inline
double GSS_generator::get_nominal_sigma() const
{
    return sigma_n;
}

//----------------------------------------------------------------------------------------

inline
void GSS_generator::check_params() const
{
    if(O < 1)
    {
        KJB_THROW_2(kjb::Illegal_argument, "The number of octaves must be positive.");
    }

    if(S < 1)
    {
        KJB_THROW_2(kjb::Illegal_argument, "The number of levels must be positive.");
    }

    if(s_min >= s_max || s_min >= S)
    {
        KJB_THROW_2(kjb::Illegal_argument, "The minimum level must be smaller than the maximum level.");
    }

    if(sigma_0 < 0.0 || sigma_n < 0.0)
    {
        KJB_THROW_2(kjb::Illegal_argument, "The initial and nominal sigmas must be non-negative.");
    }

    if(sigma_n >= sigma_0 * std::pow(2, o_min + static_cast<double>(s_min) / S))
    {
        KJB_THROW_2(kjb::Illegal_argument, "The nominal smoothing sigma exceeds that of the minimum scale smoothing.");
    }
}


#endif /*GSS_INCLUDED_H_ */

