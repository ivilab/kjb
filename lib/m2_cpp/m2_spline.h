
/* $Id: m2_spline.h 18244 2014-11-19 23:03:32Z ksimek $ */

#ifndef KJB_SPLINE_H
#define KJB_SPLINE_H

#include <l_cpp/l_cloneable.h>
#include <m_cpp/m_vector.h>
#include <g_cpp/g_quaternion.h>
#include <vector>

/**
 * @file 
 *
 * @brief Definitions of spline classes.
 *
 * Classes here are 
 * mathematical models; for renderable graphics splines, see 
 * gr_spline.h
 *
 * @todo 
 * - Subdivisions of nurbs curves and polybezier curves
 * - Fitting splines to data points
 * - Nurbs surfaces
 */

namespace kjb {

/**
 * Abstract type for spline curves
 *
 * @author Kyle Simek
 */
class Spline_curve : public virtual Cloneable
{
public:
    virtual Vector evaluate(double u) const = 0;
    virtual Vector gradient(double u) const = 0;
};

/**
 * Abstract type for spline surfaces
 *
 * @author Kyle Simek
 */
class Spline_surface : public virtual Cloneable
{
public:
    virtual Vector evaluate(double s, double t) const = 0;
};


/**
 * Non-rational B-spline class.    
 *
 * @see Gl_nurbs_curve
 * @author Kyle Simek
 */
class Nurbs_curve : public Spline_curve
{
public:
    Nurbs_curve();
    Nurbs_curve(uint num_knots, const float* knots, uint degree, const std::vector<Vector>& ctl_points);
    Nurbs_curve(const Nurbs_curve& src);
    virtual ~Nurbs_curve();

    void insert_at(int index, const Vector& control_point);

    virtual Nurbs_curve& operator=(const Nurbs_curve& src);
    virtual Nurbs_curve* clone() const;

    kjb::Vector evaluate(double u) const;
    kjb::Vector gradient(double u) const;
    
    bool has_zero_length() const;
protected:
    uint _order;

    uint _num_knots;
    float* _knots;

    std::vector<Vector> _ctl_points;

private:
    double _dN(int n, int d, double t) const;
    double _N(int n, int d, double t) const;
};



/**
 *  Non-rational B-spline surface.
 *
 * @see Gl_nurbs_surface
 * @author Kyle Simek
 */
class Nurbs_surface : public Spline_surface 
{
    public:
        Nurbs_surface();
        Nurbs_surface(
            uint num_knots_s,
            const float* knots_s,
            uint num_knots_t,
            const float* knots_t,
            uint degree_u,
            uint degree_v,
            const std::vector<std::vector<Vector> >& ctl_points);

        Nurbs_surface( const Nurbs_surface& other );
        ~Nurbs_surface();


        Vector gradient_s(double s, double t) const;
        Vector gradient_t(double s, double t) const;
        Vector normal(double s, double t) const;

        Vector evaluate(double s, double t) const; 


        virtual Nurbs_surface& operator=(const Nurbs_surface& src);
        virtual Nurbs_surface* clone() const;

        virtual double& operator()(uint u, uint v, uint d)
        {
            return _ctl_points[u][v][d];
        }
         
        virtual double operator()(uint u, uint v, uint d) const
        {
            return _ctl_points[u][v][d];
        }
    protected:
        uint _order_s;
        uint _order_t;
        uint _num_knots_s;
        uint _num_knots_t;
        float* _knots_s;
        float* _knots_t;
        std::vector< std::vector<Vector> > _ctl_points;

    private:
        void _print() const;

}; // class Nurbs_surface



/**
 * Bezier curve of arbitrary dimension.  
 * @author Kyle Simek
 */
class Bezier_curve : public Spline_curve
{
public:
    Bezier_curve(int degree = 3, int dimension = 3);
    Bezier_curve(const Bezier_curve& src);
    virtual ~Bezier_curve(){}

    Bezier_curve& operator=(const Bezier_curve& src);
    virtual Bezier_curve* clone() const;

    /** 
     * @warning The current implementation of evaluate() currently 
     * converts to a nurbs object and then evaluates the nurbs object.
     * As a result, this may be slower than expected.
     */
    kjb::Vector evaluate(double u) const;
    kjb::Vector gradient(double u) const;

    void set_control_point(int index, const Vector& pt);

    const kjb::Vector& get_control_point(int index) const;

    /** @brief Convert to a mathematically equivalent nurbs curve. */
    Nurbs_curve to_nurbs() const;
private:
    std::vector<Vector> _ctl_points;
};


/** 
 * Sequence of cubic bezier curves of arbitrary dimension, connected 
 * end-to-end.  In 2D, this is equivalent to paths in common graphics
 * packages, and the API is designed to mimick this representation, 
 * using "handles" and "curve points" instead of the more generic 
 * "control points".
 *
 * @see Gl_polybezier_curve
 * @author Kyle Simek
 */
class Polybezier_curve : public Spline_curve
{
public:
    enum Point_type { CURVE_POINT = 0, HANDLE_2_POINT = 1, HANDLE_1_POINT = 2 };

    Polybezier_curve(int dimension);
    Polybezier_curve(const std::vector<kjb::Vector>& control_points);

    Polybezier_curve* clone() const;

    static void decode_control_point(size_t i, size_t& curve_i, Point_type& point_type);
    static size_t encode_control_point(size_t curve_i, Point_type point_type);

    size_t size() const;
    size_t dimension() const;

    void swap(Polybezier_curve& other);

    /** 
     * @warning The current implementation of evaluate() currently 
     * converts to a nurbs object and then evaluates the nurbs object.
     * As a result, this may be slower than expected.
     */
    kjb::Vector evaluate(double u) const;
    kjb::Vector gradient(double u) const;
    
    void insert_curve_point(int index, const kjb::Vector& curve_pt);
    void insert(int index, const kjb::Vector& curve_pt, const kjb::Vector& handle_pt_1, const kjb::Vector& handle_pt_2);

    void set_curve_point(int index, const kjb::Vector& pt);
    void set_handle_point_1(int index, const kjb::Vector& pt);
    void set_handle_point_2(int index, const kjb::Vector& pt);


    void delete_curve_point(int index);

    void reverse();

    /** set handle point based on location of 2nd handle point so curve is infinituely differentiable at the curve point */
    void symmetrize_handle_1(int index);
    void symmetrize_handle_2(int index);

    kjb::Vector& get_curve_point(int index);
    const kjb::Vector& get_curve_point(int index) const;

    kjb::Vector& get_handle_point_1(int index);
    const kjb::Vector& get_handle_point_1(int index) const;

    kjb::Vector& get_handle_point_2(int index);
    const kjb::Vector& get_handle_point_2(int index) const;

    kjb::Vector& get_control_point(int index);
    const kjb::Vector& get_control_point(int index) const;

    /** @brief Convert to a mathematically equivalent nurbs curve. */
    virtual Nurbs_curve to_nurbs() const;

#ifdef DEBUG
    /** @brief Output as a string compatible with the SVG specification for "path" type objects. */
    void print_svg() const;
#endif
    friend bool operator==(const Polybezier_curve& op1, const Polybezier_curve& op2);
private:
    std::vector<kjb::Vector> _curve_points;
    std::vector<kjb::Vector> _handle_1_points;
    std::vector<kjb::Vector> _handle_2_points;

    int _dimension;
};

Polybezier_curve subdivide(const Polybezier_curve& curve, double u);

bool operator==(const Polybezier_curve& op1, const Polybezier_curve& op2);

}

#endif
