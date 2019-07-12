/* $Id */

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
|  Author: Luca Del Pero
* =========================================================================== */


#include "edge_cpp/features_manager.h"
#include "edge_cpp/vanishing_point_detector.h"
#include <sstream>
#include <string>

using namespace kjb;

/**
 *  Constructs a features manager. All options are set to default values.
 *  All the requested features will be detected
 *
 *  @param img The image to detect features from
 *  @param idetect_edges set it to true if you want edges to be detected
 *  @param ifit_edge_segments set it to true if you want to fit line segments
 *         to the detected edges. It will raise an exception if no edges are
 *         available
 *  @param icreate_manhattan_world set it to true if you want Manahttan scene
 *         features to be detected (set of three orthogonal vanishing points)
 *
 *  */
Features_manager::Features_manager
(
    const kjb::Image & img,
    bool idetect_edges,
    bool ifit_edge_segments,
    bool icreate_manhattan_world
)
: Readable(), Writeable(),
 edges(0), edge_segments(0), manhattan_world(0),
 _edges_available(false),_edge_segments_available(false),
 _manhattan_world_available(false)
{
    set_edge_detection_parameters
    (
        FM_DEFAULT_EDGE_SIGMA,
        FM_DEFAULT_EDGE_BEGIN,
        FM_DEFAULT_EDGE_END,
        FM_DEFAULT_EDGE_PADDING,
        true
    );
    set_manhattan_world_parameters
    (
        FM_DEFAULT_VP_SUCCESS_PROBABILITY,
        FM_DEFAULT_VP_ASSIGNMENT_THRESHOLD
    );
    detect_features(idetect_edges, ifit_edge_segments, icreate_manhattan_world, img);
}

/**
 *  Constructs a features manager. All options are set to default values.
 *  All the requested features will be detected
 *
 *  @param img_path The path to the image to detect features from
 *  @param idetect_edges set it to true if you want edges to be detected
 *  @param ifit_edge_segments set it to true if you want to fit line segments
 *         to the detected edges. It will raise an exception if no edges are
 *         available
 *  @param icreate_manhattan_world set it to true if you want Manahttan scene
 *         features to be detected (set of three orthogonal vanishing points)
 *
 *  */
Features_manager::Features_manager
(
    bool read_image,
    const std::string & img_path,
    bool idetect_edges,
    bool ifit_edge_segments,
    bool icreate_manhattan_world
)
: Readable(), Writeable(),
 edges(0), edge_segments(0), manhattan_world(0),
 _edges_available(false),_edge_segments_available(false),
 _manhattan_world_available(false)
{
    set_edge_detection_parameters
    (
        FM_DEFAULT_EDGE_SIGMA,
        FM_DEFAULT_EDGE_BEGIN,
        FM_DEFAULT_EDGE_END,
        FM_DEFAULT_EDGE_PADDING,
        true
    );
    set_manhattan_world_parameters
    (
        FM_DEFAULT_VP_SUCCESS_PROBABILITY,
        FM_DEFAULT_VP_ASSIGNMENT_THRESHOLD
    );
    detect_features(idetect_edges, ifit_edge_segments, icreate_manhattan_world, img_path);
}

/**
 *  Constructs a features manager.
 *  All the requested features will be detected

 * @param img The image to detect features from *
 * @param iblurring_sigma Gaussian blurring sigma.  Determines scale of edges to detect.
 * @param ibegin_threshold Starting edge threshold hysteresis in the Canny edge detection algorithm.
 *        Lower value gives more edges.
 * @param iend_threshold Ending edge threshold for hysteresis in the Canny edge detection algorithm.
 *        Lower value gives longer edges.
 * @param ipadding Amount of padding to add to images before detecting edges.
 *                Images are padded by repeating the values occurring at image boundaries.
 *                Adding padding can prevent edges detected at image boundary,
 *                but often this is set to zero (default).
 * @param iuse_fourier Specifies whether to use Fast Fourier transform for convolution or not
 * @param idetect_edges set it to true if you want edges to be detected
 * @param ifit_edge_segments set it to true if you want to fit line segments
 *        to the detected edges. It will raise an exception if no edges are
 *        available
 * @param icreate_manhattan_world set it to true if you want Manahttan scene
 *        features to be detected (set of three orthogonal vanishing points)
 * @param  ivanishing_point_detection_success_probability
 *         The probability that Ransac will successfully detect the
 *         vanishing points
 * @param  ioutlier_threshold_for_vanishing_points_assignment
 *         the threshold above which a line segment is considered an outlier.
 *         We try to assign each segment to a vanishing point, and if none
 *         of them gives a penalty smaller than the threshold the segment
 *         will be considered as an outlier
 *
 */
Features_manager::Features_manager
(
    const kjb::Image & img,
    float iblurring_sigma,
    float ibegin_threshold,
    float iend_threshold,
    unsigned int ipadding,
    bool iuse_fourier,
    double ivanishing_point_detection_success_probability,
    double ioutlier_threshold_for_vanishing_points_assignment,
    bool idetect_edges,
    bool ifit_edge_segments,
    bool icreate_manhattan_world
) : Readable(), Writeable(),
    edges(0), edge_segments(0), manhattan_world(0),
    _edges_available(false),_edge_segments_available(false),
    _manhattan_world_available(false)
{
    set_edge_detection_parameters
    (
        iblurring_sigma,
        ibegin_threshold,
        iend_threshold,
        ipadding,
        iuse_fourier
    );

    set_manhattan_world_parameters
    (
        ivanishing_point_detection_success_probability,
        ioutlier_threshold_for_vanishing_points_assignment
    );
    detect_features(idetect_edges, ifit_edge_segments, icreate_manhattan_world, img);

}

/** Constructs a features manager. If one input pointer
 * is NULL, it will be assumed that that feature is not available.
 * No deep copy will be done, the memory management for the
 * input objects will be transfered to this class
 *
 * @param iedges a pointer to the edges
 * @param iedge_segments a pointer to the edge segments
 * @param imanhattan_world a pointer to the Manhattan world
 */
Features_manager::Features_manager
(
    kjb::Edge_set * iedges,
    Edge_segment_set * iedge_segments,
    Manhattan_world * imanhattan_world
)
{
    set_edge_detection_parameters
    (
        FM_DEFAULT_EDGE_SIGMA,
        FM_DEFAULT_EDGE_BEGIN,
        FM_DEFAULT_EDGE_END,
        FM_DEFAULT_EDGE_PADDING,
        true
    );

    set_manhattan_world_parameters
    (
        FM_DEFAULT_VP_SUCCESS_PROBABILITY,
        FM_DEFAULT_VP_ASSIGNMENT_THRESHOLD
    );

    if(iedges != NULL)
    {
        _edges_available = true;
        edges = iedges;
    }
    else
    {
        _edges_available = false;
        edges = NULL;
    }

    if(iedge_segments != NULL)
    {
        if(!_edges_available)
        {
            KJB_THROW_2(KJB_error, "Features manager, cannot have edge segments without edges");
        }
        _edge_segments_available = true;
        edge_segments = iedge_segments;
    }
    else
    {
        _edge_segments_available = false;
        edge_segments = NULL;
    }

    if(imanhattan_world != NULL)
    {
        if(!_edges_available)
        {
            KJB_THROW_2(KJB_error, "Features manager, cannot have Manhattan world without edge segments");
        }
        _manhattan_world_available = true;
        manhattan_world = imanhattan_world;
    }
    else
    {
        _manhattan_world_available = false;
        manhattan_world = NULL;
    }
}


/*
 * @param in The input stream to read this Features_manager from
 */
void Features_manager::read(std::istream& in)
{
    using std::istringstream;

    const char* field_value;

    delete manhattan_world;
    delete edge_segments;
    delete edges;

    _edges_available = false;
    _edge_segments_available = false;
    _manhattan_world_available = false;

    /** Read the options */
    if (!(field_value = read_field_value(in, "blurring_sigma")))
    {
        KJB_THROW_2(Illegal_argument, "Features manager set, could not read edge set");
    }
    istringstream ist(field_value);
    ist >> blurring_sigma;
    if (ist.fail() )
    {
        KJB_THROW_2(Illegal_argument, "Invalid features manager file format, invalid blurring sigma");
    }
    ist.clear(std::ios_base::goodbit);

    if (!(field_value = read_field_value(in, "begin_threshold")))
    {
        KJB_THROW_2(Illegal_argument, "Features manager set, could not read edge set");
    }
    ist.str(field_value);
    ist >> begin_threshold;
    if (ist.fail() )
    {
        KJB_THROW_2(Illegal_argument, "Invalid features manager file format, invalid begin threshold");
    }
    ist.clear(std::ios_base::goodbit);

    if (!(field_value = read_field_value(in, "end_threshold")))
    {
        KJB_THROW_2(Illegal_argument, "Features manager set, could not read end threshold");
    }
    ist.str(field_value);
    ist >> end_threshold;
    if (ist.fail() )
    {
        KJB_THROW_2(Illegal_argument, "Invalid features manager file format, invalid end threshold");
    }
    ist.clear(std::ios_base::goodbit);

    if (!(field_value = read_field_value(in, "padding")))
    {
        KJB_THROW_2(Illegal_argument, "Features manager set, could not read padding");
    }
    ist.str(field_value);
    ist >> padding;
    if (ist.fail() )
    {
        KJB_THROW_2(Illegal_argument, "Invalid features manager file format, invalid padding");
    }
    ist.clear(std::ios_base::goodbit);

    if (!(field_value = read_field_value(in, "use_fourier")))
    {
        KJB_THROW_2(Illegal_argument, "Features manager set, could not read use fourier");
    }
    ist.str(field_value);
    ist >> use_fourier;
    if (ist.fail() )
    {
        KJB_THROW_2(Illegal_argument, "Invalid features manager file format, invalid use fourier");
    }
    ist.clear(std::ios_base::goodbit);

    if (!(field_value = read_field_value(in, "Vpd_success")))
    {
        KJB_THROW_2(Illegal_argument, "Features manager set, could not read vanishing points detection success probability");
    }
    ist.str(field_value);
    ist >> vanishing_point_detection_success_probability;
    if (ist.fail() )
    {
        KJB_THROW_2(Illegal_argument, "Invalid features manager file format, invalid vanishing points detection success probability");
    }
    ist.clear(std::ios_base::goodbit);

    if (!(field_value = read_field_value(in, "Vpd_outlier_threshold")))
    {
        KJB_THROW_2(Illegal_argument, "Features manager set, could not read vanishing points detection outlier threshold");
    }
    ist.str(field_value);
    ist >> outlier_threshold_for_vanishing_points_assignment;
    if (ist.fail() )
    {
        KJB_THROW_2(Illegal_argument, "Invalid features manager file format, invalid vanishing points detection outlier threshold");
    }
    ist.clear(std::ios_base::goodbit);

    /** Now read the features */
    if (!(field_value = read_field_value(in, "edge_set")))
    {
        KJB_THROW_2(Illegal_argument, "Features manager set, could not read edge set");
    }
    ist.str(field_value);
    ist >> _edges_available;
    if (ist.fail())
    {
        KJB_THROW_2(Illegal_argument, "Invalid features manager file format, missing edge information");
    }
    ist.clear(std::ios_base::goodbit);
    if(_edges_available)
    {
        bool read_from_file = false;
        if (!(field_value = read_field_value(in, "read_from_other_file")))
        {
            KJB_THROW_2(Illegal_argument, "Features manager set, could not read edge segments");
        }
        ist.str(field_value);
        ist >> read_from_file;
        if (ist.fail())
        {
            KJB_THROW_2(Illegal_argument, "Invalid features manager file format, missing edge information");
        }
        ist.clear(std::ios_base::goodbit);

        if(read_from_file)
        {
            std::string filename;
            if (!(field_value = read_field_value(in, "filename")))
            {
                KJB_THROW_2(Illegal_argument, "Features manager set, could not read edge segments");
            }
            ist.str(field_value);
            ist >> filename;
            if (ist.fail())
            {
                KJB_THROW_2(Illegal_argument, "Invalid features manager file format, missing edge information");
            }
            ist.clear(std::ios_base::goodbit);
            edges = new Edge_set(filename.c_str());
        }
        else
        {
            size_t length;
            if (!(field_value = read_field_value(in, "size")))
            {
                KJB_THROW_2(Illegal_argument, "Features manager set, could not read edge set size");
            }
            ist.str(field_value);
            ist >> length;
            if (ist.fail())
            {
                KJB_THROW_2(Illegal_argument, "Invalid features manager file format, missing edge information");
            }
            ist.clear(std::ios_base::goodbit);

            char * buffer = new char[length];
            in.read(buffer, length);

            kjb_c::Edge_set * edgeset = 0;
            if(kjb_c::unserialize_edge_set(&edgeset, buffer) != kjb_c::NO_ERROR )
            {
                delete[] buffer;
                KJB_THROW_2(Illegal_argument, "Invalid features manager file format, missing edge information");
            }

            edges = new Edge_set(edgeset);
            delete[] buffer;
        }
    }

    if (!(field_value = read_field_value(in, "edge_segments")))
    {
        KJB_THROW_2(Illegal_argument, "Features manager set, could not read edge segments");
    }
    ist.str(field_value);
    ist >> _edge_segments_available;
    if (ist.fail())
    {
        KJB_THROW_2(Illegal_argument, "Invalid features manager file format, missing edge segments information");
    }
    ist.clear(std::ios_base::goodbit);

    if(_edge_segments_available)
    {
        bool read_from_file = false;
        if (!(field_value = read_field_value(in, "read_from_other_file")))
        {
            KJB_THROW_2(Illegal_argument, "Features manager set, could not read edge segments");
        }
        ist.str(field_value);
        ist >> read_from_file;
        if (ist.fail())
        {
            KJB_THROW_2(Illegal_argument, "Invalid features manager file format, missing edge information");
        }
        ist.clear(std::ios_base::goodbit);

        if(read_from_file)
        {
            std::string filename;
            if (!(field_value = read_field_value(in, "filename")))
            {
                KJB_THROW_2(Illegal_argument, "Features manager set, could not read edge segments");
            }
            ist.str(field_value);
            ist >> filename;
            if (ist.fail())
            {
                KJB_THROW_2(Illegal_argument, "Invalid features manager file format, missing edge information");
            }
            ist.clear(std::ios_base::goodbit);

            edge_segments = new Edge_segment_set(edges, filename.c_str());

        }
        else
        {
            /** This will throw an exception if edges and edge segments do not match */
            edge_segments = new Edge_segment_set(edges, in);
        }
    }
    ist.clear(std::ios_base::goodbit);

    if (!(field_value = read_field_value(in, "manhattan_world")))
    {
        KJB_THROW_2(Illegal_argument, "Features manager set, could not read edge segments");
    }
    ist.str(field_value);
    ist >> _manhattan_world_available;
    if (ist.fail())
    {
        KJB_THROW_2(Illegal_argument, "Invalid features manager file format, missing Manhattan world information");
    }
    ist.clear(std::ios_base::goodbit);

    if(_manhattan_world_available)
    {
        if(!_edge_segments_available)
        {
            KJB_THROW_2(Illegal_argument, "Invalid features manager file format, cannot create"
                    "Manhattan world without edge_segments");
        }

        bool read_from_file = false;
        if (!(field_value = read_field_value(in, "read_from_other_file")))
        {
            KJB_THROW_2(Illegal_argument, "Features manager set, could not read edge segments");
        }
        ist.str(field_value);
        ist >> read_from_file;
        if (ist.fail())
        {
            KJB_THROW_2(Illegal_argument, "Invalid features manager file format, missing manhattan world file information");
        }
        ist.clear(std::ios_base::goodbit);
        if(read_from_file)
        {
            std::string filename;
            if (!(field_value = read_field_value(in, "filename")))
            {
                KJB_THROW_2(Illegal_argument, "Features manager set, could not read Manhattan world");
            }
            ist.str(field_value);
            ist >> filename;
            if (ist.fail())
            {
                KJB_THROW_2(Illegal_argument, "Invalid features manager file format, missing Manhattan world information");
            }
            ist.clear(std::ios_base::goodbit);
            
            manhattan_world = new Manhattan_world(filename.c_str(), *edge_segments, outlier_threshold_for_vanishing_points_assignment);

        }
        else
        {
            manhattan_world = new Manhattan_world(in, *edge_segments, outlier_threshold_for_vanishing_points_assignment);
        }

    }

}

/** @brief Writes this Features_manager to an output stream.
 * For now it writes everything in the same file.
 * I'll add the capability to write each feature on a different
 * file shortly */
void Features_manager::write(std::ostream& out) const
{
    /** Write the options */
    out << "blurring_sigma: " << blurring_sigma  << "\n";
    out << "begin_threshold: " << begin_threshold << "\n";
    out << "end_threshold: " << end_threshold  << "\n";
    out << "padding: " << padding   << "\n";
    out << "use_fourier: " << use_fourier << "\n";

    out << "Vpd_success: " << vanishing_point_detection_success_probability << "\n";
    out << "Vpd_outlier_threshold: " << outlier_threshold_for_vanishing_points_assignment << "\n";

    /** Write the features */
    out << "edge_set: " << _edges_available << "\n";
    if(_edges_available)
    {
        char * _buffer = 0;
        size_t length;
        out << "read_from_other_file: " << false << "\n";
        if( kjb_c::serialize_edge_set(edges->c_ptr(), &_buffer, &length) != kjb_c::NO_ERROR)
        {
            KJB_THROW_2(Serialization_error, "Could not serialize edge set");
        }
        out << "size:  " << length << "\n";
        out.write(_buffer, length);
        kjb_c::kjb_free(_buffer);
    }

    out << "edge_segments: " << _edge_segments_available << "\n";
    if(_edge_segments_available)
    {
        out << "read_from_other_file: " << false << "\n";
        edge_segments->write(out);
    }

    out << "manhattan_world: " << _manhattan_world_available << "\n";
    if(_manhattan_world_available)
    {
        out << "read_from_other_file: " << false << "\n";
        manhattan_world->write(out);
    }
}

/**
 * Sets the parameters needed for edge detection

 * @param iblurring_sigma Gaussian blurring sigma.  Determines scale of edges to detect.
 * @param ibegin_threshold Starting edge threshold hysteresis in the Canny edge detection algorithm.
 *        Lower value gives more edges.
 * @param iend_threshold Ending edge threshold for hysteresis in the Canny edge detection algorithm.
 *        Lower value gives longer edges.
 * @param ipadding Amount of padding to add to images before detecting edges.
 *                Images are padded by repeating the values occurring at image boundaries.
 *                Adding padding can prevent edges detected at image boundary,
 *                but often this is set to zero (default).
 * @param iuse_fourier Specifies whether to use Fast Fourier transform for convolution or not
 */
void Features_manager::set_edge_detection_parameters
(
  float iblurring_sigma,
  float ibegin_threshold,
  float iend_threshold,
  unsigned int ipadding,
  bool iuse_fourier
)
{
    if(iblurring_sigma < DBL_EPSILON)
    {
        KJB_THROW_2(Illegal_argument, "Negative or too small blurring sigma for edge detection");
    }
    blurring_sigma = iblurring_sigma;

    if(ibegin_threshold < 0)
    {
        KJB_THROW_2(Illegal_argument, "Negative begin threshold for Canny edge detection");
    }
    begin_threshold = ibegin_threshold;

    if(iend_threshold < 0)
    {
        KJB_THROW_2(Illegal_argument, "Negative end threshold for Canny edge detection");
    }
    if(ibegin_threshold < iend_threshold)
    {
        KJB_THROW_2(Illegal_argument, "Begin threshold for Canny edge detection must be bigger than end threshold");
    }
    end_threshold = iend_threshold;

    padding = ipadding;

    use_fourier = iuse_fourier;
}

/** Sets the parameters needed for extracting Manhattan features
 *
 * @param  ivanishing_point_detection_success_probability
 *         The probability that Ransac will successfully detect the
 *         vanishing points
 * @param  ioutlier_threshold_for_vanishing_points_assignment
 *         the threshold above which a line segment is considered an outlier.
 *         We try to assign each segment to a vanishing point, and if none
 *         of them gives a penalty smaller than the threshold the segment
 *         will be considered as an outlier
 */
void Features_manager::set_manhattan_world_parameters
(
    double ivanishing_point_detection_success_probability,
    double ioutlier_threshold_for_vanishing_points_assignment
)
{
    if(ivanishing_point_detection_success_probability <= 0 || ivanishing_point_detection_success_probability >= 1)
    {
        KJB_THROW_2(Illegal_argument, "RANSAC probability of finding vanishing points must be between 0 and 1");
    }
    vanishing_point_detection_success_probability = ivanishing_point_detection_success_probability;
    outlier_threshold_for_vanishing_points_assignment = ioutlier_threshold_for_vanishing_points_assignment;
}

/**
 *
 * Detect image edges using the Canny algorithm. In the future
 * we will add other ways of detecting edges, ie the Berkeley
 * edge detector
 *
 * @param img The image to detect edges from
 */
void Features_manager::detect_edges(const kjb::Image & img)
{
    Canny_edge_detector edge_detector(blurring_sigma, begin_threshold, end_threshold, padding, use_fourier);
    edges = edge_detector.detect_edges(img);
    edges->remove_short_edges(20);
    edges->break_edges_at_corners(0.85, 7);
    edges->break_edges_at_corners(0.97, 60);
    edges->remove_short_edges(10);
    _edges_available = true;

}

/**
 * Fits edge segments to the detected edges
 */
void Features_manager::fit_edge_segments_to_edges()
{
    if(!_edges_available)
    {
        KJB_THROW_2(KJB_error, "Features manager, cannot fit edge segments if edges are not available");
    }
    edge_segments = new Edge_segment_set(edges);
    _edge_segments_available = true;
}

/**
 * Detects Manhattan features (vanishing points mainly) from an image.
 * If this features manager contains some fitted line segments,
 * they will be assigned to the vanishing point they converge
 * to
 *
 * @param img The image to detect Manhattan features from
 */
bool Features_manager::create_manhattan_world(const kjb::Image & img)
{
    std::vector<Vanishing_point> vpts;
    double focal_length;
    double success = robustly_estimate_vanishing_points(vpts, focal_length, img,vanishing_point_detection_success_probability, true);
    _manhattan_world_available = false;
    if(success)
    {
        manhattan_world = new Manhattan_world(vpts, focal_length);
        _manhattan_world_available = true;
        if(_edge_segments_available)
        {
            manhattan_world->assign_segments_to_vpts(*edge_segments, outlier_threshold_for_vanishing_points_assignment);
            manhattan_world->create_corners();
        }
    }
    return success;
}

/**
 * Detects Manhattan features (vanishing points mainly) from an image.
 * If this features manager contains some fitted line segments,
 * they will be assigned to the vanishing point they converge
 * to
 *
 * @param img_path The path to the image to detect Manhattan features from
 */
bool Features_manager::create_manhattan_world(const std::string & img_path)
{
    std::vector<Vanishing_point> vpts;
    double focal_length;
    bool success = detect_vanishing_points(vpts, focal_length, img_path);
    _manhattan_world_available = false;
    if(success)
    {
        manhattan_world = new Manhattan_world(vpts, focal_length);
        _manhattan_world_available = true;
        if(_edge_segments_available)
        {
            manhattan_world->assign_segments_to_vpts(*edge_segments, outlier_threshold_for_vanishing_points_assignment);
            manhattan_world->create_corners();
        }
    }
    return success;
}

/** Detects features from an image
 *
 * @param idetect_edges specifies whether to do edge detection or not
 * @param ifit_edge_segments specifies whether to fit line segments
 *        to the detected edges (it will raise an exception if it is
 *        true and there are no edges available)
 * @param icreate_manhattan_world specifies whether to detect features
 *        typical to Manahttan scenes (3 orthogonal vanishing points.
 *        If edge segments are available, they will be assigned to
 *        the vanishing point they converge to
 * @param img The input image to detect features from.
 */
bool Features_manager::detect_features
(
    bool idetect_edges,
    bool ifit_edge_segments,
    bool icreate_manhattan_world,
    const kjb::Image & img
)
{
    using namespace std;
    if(idetect_edges)
    {
        detect_edges(img);
    }

    if(ifit_edge_segments)
    {
        fit_edge_segments_to_edges();
        remove_frame_segments();
    }

    if(icreate_manhattan_world)
    {
        return create_manhattan_world(img);
    }

    return true;
}

/** Detects features from an image
 *
 * @param idetect_edges specifies whether to do edge detection or not
 * @param ifit_edge_segments specifies whether to fit line segments
 *        to the detected edges (it will raise an exception if it is
 *        true and there are no edges available)
 * @param icreate_manhattan_world specifies whether to detect features
 *        typical to Manahttan scenes (3 orthogonal vanishing points.
 *        If edge segments are available, they will be assigned to
 *        the vanishing point they converge to
 * @param img_path The path to the input image to detect features from.
 */
bool Features_manager::detect_features
(
    bool idetect_edges,
    bool ifit_edge_segments,
    bool icreate_manhattan_world,
    const std::string & img_path
)
{
    using namespace std;

    Image img(img_path);
    if(idetect_edges)
    {
        detect_edges(img);
    }

    if(ifit_edge_segments)
    {
        fit_edge_segments_to_edges();
        remove_frame_segments();
    }

    if(icreate_manhattan_world)
    {
        return create_manhattan_world(img_path);
    }

    return true;
}

void Features_manager::set_manhattan_focal_length(double ifocal)
{
    if(!_manhattan_world_available)
    {
        KJB_THROW_2(KJB_error,"Manhattan world is not available, cannot set focal length");
    }
    manhattan_world->set_focal_length(ifocal);
}

void Features_manager::set_manhattan_world(Manhattan_world * mw)
{
    if(_manhattan_world_available)
    {
        delete manhattan_world;
    }
    _manhattan_world_available = true;
    manhattan_world = mw;
    if(_edge_segments_available)
    {
        manhattan_world->assign_segments_to_vpts(*edge_segments, outlier_threshold_for_vanishing_points_assignment);
        manhattan_world->create_corners();
    }
}

void Features_manager::remove_frame_segments()
{
    if(_edge_segments_available)
    {
        edge_segments->remove_frame_segments(edges->get_num_rows(), edges->get_num_cols(), *edges);
    }
}

kjb::Features_manager * kjb::detect_hoiem_features_manager
(
    const std::string & img_path
)
{
    std::vector<Vanishing_point> vpts;
    double focal_length;
	if(!kjb::detect_vanishing_points(vpts, focal_length, img_path))
	{
		KJB_THROW_2(KJB_error, "Could not detect vanishing points");
	}
	Line_segment_set segments;
    Edge_segment_set ess;
    detect_long_connected_segments(segments, img_path, 10);
    Image img(img_path);
    kjb::Edge_set * hes =  segments.convert_to_edge_set(img.get_num_rows(), img.get_num_cols());
    Edge_segment_set * ees = new Edge_segment_set;
	if(segments.size() != hes->num_edges())
	{
		KJB_THROW_2(kjb::KJB_error, "Edge_set and edges do not agree!!!");
	}
	for(unsigned int i = 0; i < segments.size(); i++)
	{
		ees->add_segment( Edge_segment(hes->get_edge(i), segments.get_segment(i)) );
	}
	ees->remove_overlapping_segments(*hes, 0.05, 5);
	kjb::Edge_set * hes2 =  ees->convert_to_edge_set(img.get_num_rows(), img.get_num_cols());
	Edge_segment_set * ees2 = new Edge_segment_set();
	if(ees->size() != hes2->num_edges())
	{
		KJB_THROW_2(kjb::KJB_error, "Edge_set and edges do not agree!!!");
	}
	for(unsigned int i = 0; i < ees->size(); i++)
	{
		ees2->add_segment( Edge_segment(hes2->get_edge(i), ees->get_segment(i)) );
	}
	delete ees;
	delete hes;
    ees2->remove_frame_segments(hes2->get_num_rows(), hes2->get_num_cols(),
    *hes2);
	Manhattan_world * mw = new Manhattan_world(*ees2, vpts, focal_length);
	return new Features_manager(hes2, ees2, mw);

}
