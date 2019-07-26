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
|     Ernesto Brau
|
* =========================================================================== */

/* $Id$ */

#ifndef PT_SCENE_VIEWER
#define PT_SCENE_VIEWER

#ifdef KJB_HAVE_OPENGL
#include <people_tracking_cpp/pt_scene.h>
#include <people_tracking_cpp/pt_target.h>
#include <people_tracking_cpp/pt_detection_box.h>
#include <people_tracking_cpp/pt_data.h>
#include <people_tracking_cpp/pt_util.h>
#include <flow_cpp/flow_integral_flow.h>
#include <detector_cpp/d_deva_facemark.h>
#include <m_cpp/m_vector.h>
#include <gr_cpp/gr_glut.h>
#include <gr_cpp/gr_offscreen.h>
#include <l_cpp/l_exception.h>
#include <map>
#include <boost/bind.hpp>
#include <boost/optional.hpp>

namespace kjb {

namespace pt {

class Scene_viewer
{
private:
    struct Render_style
    {
        Vector color;
        double line_width;

        Render_style(const Vector& col, double lw) :
            color(col), line_width(lw) {}
    };

public:
    static const Render_style DEFAULT_GROUND_STYLE;
    static const Render_style DEFAULT_TARGET_STYLE;
    static const Render_style DEFAULT_DATA_STYLE;
    static const Vector DEFAULT_TEXT_COLOR;
    static const size_t DEFAULT_WIDTH = 300;
    static const size_t DEFAULT_HEIGHT = 300;

public:
    /** @brief  Initialize this viewer with a scene. */
    Scene_viewer
    (
        const Scene& scene,
        double width,
        double height,
        bool show_window = true
    );

#ifdef KJB_HAVE_GLUT
    ~Scene_viewer()
    {
        if(m_glwin)
        {
            delete m_glwin;
        }
    }
#endif

public:
    /** @brief  Set the scene to use. */
    void set_scene(const Scene& scene);

    /** @brief  Gets the scene being used. */
    const Scene& scene() { return *m_scene_p; }

    /** @brief  Resize this viewer. */
    void resize(size_t width, size_t height);

    /** @brief  Get this viewer's width. */
    double width() const;

    /** @brief  Get this viewer's height. */
    double height() const;

    /** @brief  Set the key handling callback. */
    template<class Func>
    void set_key_callback(const Func& cb)
    {
#ifdef KJB_HAVE_GLUT
        if(m_glwin)
        {
            m_glwin->set_keyboard_callback(cb);
        }
#endif
    }

    /** @brief  Set flow features to use. */
    void set_facemarks(const Facemark_data& fm_data)
    {
        IFT(fm_data.size() == scene_length(), Illegal_argument,
            "Facemark data has incorrect number of frames");

        m_facemark_data_p = &fm_data;
    }

    /** @brief  Set flow features to use. */
    void set_flows
    (
        const std::vector<Integral_flow>& flx,
        const std::vector<Integral_flow>& fly
    )
    {
        IFT(flx.size() == fly.size() && flx.size() == scene_length() - 1,
            Illegal_argument, "Flows have incorrect number of frames");

        m_flowsx_p = &flx;
        m_flowsy_p = &fly;
    }

    /** @brief  Set lines of text to appear on scene. */
    template <class InputIterator>
    void set_text_lines(InputIterator first, InputIterator last)
    {
        m_text_lines.resize(std::distance(first, last));
        std::copy(first, last, m_text_lines.begin());
    }

    /** @brief  Add line of text to appear on scene. */
    void add_text_line(const std::string& str) { m_text_lines.push_back(str); }

    /** @brief  Clear all text; i.e., no text will appear on the scene. */
    void clear_text_lines() { m_text_lines.clear(); }

    /** @brief  Set frame images from files. */
    template <class InputIterator>
    void set_frame_images(InputIterator first, InputIterator last)
    {
        IFT(std::distance(first, last) == scene_length(),
            Illegal_argument, "Incorrect number of frame images.");

        m_frames_fps.assign(first, last);
    }

    /** @brief  Clear the frame images. */
    void clear_frame_images()
    {
        m_frames_fps.clear();
    }

    /** @brief  Set the current frame to the given value. */
    void set_frame(size_t f);

    /** @brief  Advance the current frame by the given value. */
    void advance_frame(size_t df = 1);

    /** @brief  Rewind (move backwards) the current frame by the given value. */
    void rewind_frame(size_t df = 1);

    /** @brief  Returns the current frame. */
    size_t frame() const { return m_frame; }

    /** @brief  Set an overhead view. */
    void set_alternative_camera(const Perspective_camera& cam)
    {
        *m_alt_camera = cam;
    }

    /** @brief  Set an overhead view. */
    void set_overhead_view();

    /** @brief  Return to the scene's camera. */
    void clear_alternative_camera()
    {
        m_alt_camera = boost::none;
    }

    void write_images (const std::vector<std::string>& out_fps);

public:
    /** @brief  Set whether or not this viewer draws the frame images. */
    void draw_images(bool di) { m_draw_images = di; }

    /** @brief  Set whether or not this viewer draws the ground plane. */
    void draw_ground(bool dg) { m_draw_ground = dg; }

    /** @brief  Set whether or not this viewer draws cylinders. */
    void draw_cylinders(bool dc) { m_draw_cylinders = dc; }

    /** @brief  Set whether or not this viewer draws bodies. */
    void draw_bodies(bool db) { m_draw_bodies = db; }

    /** @brief  Set whether or not this viewer draws heads. */
    void draw_heads(bool dh) { m_draw_heads = dh; }

    /** @brief  Set whether or not this viewer draws faces. */
    void draw_faces(bool df) { m_draw_faces = df; }

    /** @brief  Set whether or not this viewer draws cylinder bottoms. */
    void draw_bottoms(bool db) { m_draw_bottoms = db; }

    /** @brief  Set whether or not this viewer draws trails. */
    void draw_trails(bool dt) { m_draw_trails = dt; }

    /** @brief  Set whether or not this viewer draws objects. */
    void draw_objects(bool dj) { m_draw_objects = dj; }

    /** @brief  Set whether or not this viewer draws model boxes. */
    void draw_full_boxes(bool dfb) { m_draw_full_boxes = dfb; }

    /** @brief  Set whether or not this viewer draws body boxes. */
    void draw_body_boxes(bool dbb) { m_draw_body_boxes = dbb; }

    /** @brief  Set whether or not this viewer draws face boxes. */
    void draw_face_boxes(bool dfb) { m_draw_face_boxes = dfb; }

    /** @brief  Set whether or not this viewer draws face features. */
    void draw_face_features(bool dff) { m_draw_face_features = dff; }

    /** @brief  Set whether or not this viewer draws model vectors. */
    void draw_model_vectors(bool dmv) { m_draw_model_vectors = dmv; }

    /** @brief  Set whether or not this viewer draws face vectors. */
    void draw_face_vectors(bool dfv) { m_draw_face_vectors = dfv; }

    /** @brief  Set whether or not this viewer draws data boxes. */
    void draw_data_boxes(bool ddb) { m_draw_data_boxes = ddb; }

    /** @brief  Set whether or not this viewer draws facemarks. */
    void draw_facemarks(bool dfm) { m_draw_facemarks = dfm; }

    /** @brief  Set whether or not this viewer draws flow vectors. */
    void draw_flow_vectors(bool dfv) { m_draw_flow_vectors = dfv; }

    /** @brief  Set whether or not this viewer draws flow face vectors. */
    void draw_flow_face_vectors(bool dffv) { m_draw_flow_face_vectors = dffv; }

    /** @brief  Set whether or not this viewer draws text. */
    void draw_text(bool dt) { m_draw_text = dt; }

    /** @brief  Draw the labels of the person (only works in OpenGL mode).*/
    void draw_labels(bool dl);

    /** @brief  Set whether or not this viewer draws text. */
    void weigh_data_box_color(bool wbc) { m_weigh_box_color = wbc; }

    /** @brief  Draw 3D only things. */
    void draw_3d_mode();

    /** @brief  Draw 2D only things. */
    void draw_2d_mode();

    /** @brief  Set whether or not this viewer displays solid 3D shapes. */
    void draw_solid(bool ds) { m_draw_solid = ds; }

public:
    /** @brief  Add a style to a target. */
    void set_target_style
    (
        const Target& target,
        const Vector& color,
        double line_width,
        bool only = false
    );

    /** @brief  Add a style to the data of a target. */
    void set_target_data_style
    (
        const Target& target,
        const Vector& color,
        double line_width,
        bool only = false
    );

    /** @brief  Set the default target style. */
    void set_target_style(const Vector& color, double line_width)
    {
        m_target_style.color = color;
        m_target_style.line_width = line_width;
    }

    /** @brief  Set target styles to defaults. */
    void clear_target_styles() { m_target_styles.clear(); }

    /** @brief  Add a style to a data box. */
    void set_data_box_style
    (
        const Detection_box& dbox,
        const Vector& color,
        double line_width,
        bool only = false
    );

    /** @brief  Add a style to a facemark. */
    void set_facemark_style
    (
        const Deva_facemark& fmark,
        const Vector& color,
        double line_width,
        bool only = false
    );

    /** @brief  Set the default data style. */
    void set_data_style(const Vector& color, double line_width)
    {
        m_data_style.color = color;
        m_data_style.line_width = line_width;
    }

    /** @brief  Set box styles to defaults. */
    void clear_data_box_styles() { m_data_box_styles.clear(); }

    /** @brief  Set box styles to defaults. */
    void clear_facemark_styles() { m_facemark_styles.clear(); }

    /** @brief  Sets the default text color. */
    void set_text_color(const Vector& color) { m_text_color = color; }

    /** @brief  Builds entity_map. */
    void build_entity_map();

    /** @brief  Force redisplay on this viewer. */
#ifdef KJB_HAVE_GLUT
    void redisplay() const { if(m_glwin) m_glwin->redisplay(); }
#endif

private:
    /** @brief  Sets up GL matrices for 2D rendering. */
    void prepare_for_rendering_2d() const;

    /** @brief  Initialize GL sutff. Called from constructor. */
    void init_gl();

    /** @brief  Set up lighting. */
    void init_lighting() const;

    /** @brief  Reshape callback. */
    void reshape_scene(int w, int h) const;

    /** @brief  Renders the scene. */
    void render_scene() const;

    /* @brief   Renders the ground plane. */
    void render_ground() const;

    /* @brief   Renders the cylinders. */
    void render_cylinders() const;

    /* @brief   Renders the bodies. */
    void render_heads() const;

    /* @brief   Renders the bottoms of the cylinders. */
    void render_bottoms() const;

    /* @brief   Renders the objects in the scene. */
    void render_objects() const;

    /* @brief   Renders the trails. */
    void render_trails() const;

    /* @brief   Renders the model boxes. */
    void render_model_boxes() const;

    /* @brief   Renders the face boxes. */
    void render_face_boxes() const;

    /* @brief   Renders the model direction vectors. */
    void render_model_vectors() const;

    /* @brief   Renders the data boxes. */
    void render_data_boxes() const;

    /* @brief   Renders the facemarks. */
    void render_facemarks() const;

    /* @brief   Renders the flow vectors. */
    void render_flow_vectors() const;

    /* @brief   Renders the text. */
    void render_text() const;

    /* @brief   Render flow arrow; helper function. */
    void render_flow_arrow
    (
        const Bbox& box,
        const Visibility& vis,
        const Integral_flow& fx,
        const Integral_flow& fy
    ) const;

    /* @brief   Set current style. */
    void set_current_style(const Target& tg) const;

    /* @brief   Gets the color for a specific target. */
    const Vector& get_target_color(const Target& target) const;

    /* @brief   Gets the line width for a specific target. */
    double get_target_lwidth(const Target& target) const;

    /* @brief   Gets the color for a specific data box. */
    const Vector& get_data_box_color(const Detection_box& dbox) const;

    /* @brief   Gets the line width for a specific data box. */
    double get_data_box_lwidth(const Detection_box& dbox) const;

    /* @brief   Gets the color for a specific facemark. */
    const Vector& get_facemark_color(const Deva_facemark& fmark) const;

    /* @brief   Gets the line width for a specific facemark. */
    double get_facemark_lwidth(const Deva_facemark& fmark) const;

    /* @brief   Gets the color for a particular string. */
    Vector get_text_line_color(const std::string& str) const;

    /* @brief   Helper function: returns the number of frames in the scene. */
    size_t scene_length() const
    {
        return m_scene_p->association.get_data().size();
    }

private:
    // scene-releated members
#ifdef KJB_HAVE_GLUT
    opengl::Glut_window* m_glwin;
#endif
    const Scene* m_scene_p;
    const Facemark_data* m_facemark_data_p;
    const std::vector<Integral_flow>* m_flowsx_p;
    const std::vector<Integral_flow>* m_flowsy_p;
    std::vector<std::string> m_text_lines;
    std::vector<std::string> m_frames_fps;
    size_t m_frame;
    boost::optional<Perspective_camera> m_alt_camera;

    // what to render
    bool m_draw_images;
    bool m_draw_ground;
    bool m_draw_cylinders;
    bool m_draw_bodies;
    bool m_draw_heads;
    bool m_draw_faces;
    bool m_draw_bottoms;
    bool m_draw_trails;
    bool m_draw_objects;
    bool m_draw_full_boxes;
    bool m_draw_body_boxes;
    bool m_draw_face_boxes;
    bool m_draw_face_features;
    bool m_draw_model_vectors;
    bool m_draw_face_vectors;
    bool m_draw_data_boxes;
    bool m_draw_facemarks;
    bool m_draw_flow_vectors;
    bool m_draw_flow_face_vectors;
    bool m_draw_text;
    bool m_weigh_box_color;
    bool m_draw_solid;
    bool m_draw_labels;

    // styles
    Render_style m_ground_style;
    Render_style m_target_style;
    Render_style m_data_style;
    Vector m_text_color;
    std::map<const Target*, Render_style> m_target_styles;
    std::map<const Detection_box*, Render_style> m_data_box_styles;
    std::map<const Deva_facemark*, Render_style> m_facemark_styles;
    std::map<const Target*, Entity_id> m_entity_map;

};

}} //namespace kjb::pt

#endif /* KJB_HAVE_OPENGL */

#endif /*PT_SCENE_VIEWER */

