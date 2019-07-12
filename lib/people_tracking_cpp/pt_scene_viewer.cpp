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

#ifdef KJB_HAVE_OPENGL

#include "l/l_sys_debug.h"  /* For ASSERT */
#include "people_tracking_cpp/pt_scene_viewer.h"
#include "people_tracking_cpp/pt_scene.h"
#include "people_tracking_cpp/pt_target.h"
#include "people_tracking_cpp/pt_complete_trajectory.h"
#include "people_tracking_cpp/pt_body_2d_trajectory.h"
#include "people_tracking_cpp/pt_detection_box.h"
#include "people_tracking_cpp/pt_util.h"
#include "people_tracking_cpp/pt_association.h"
#include "people_tracking_cpp/pt_visibility.h"
#include "flow_cpp/flow_integral_flow.h"
#include "detector_cpp/d_bbox.h"
#include "detector_cpp/d_deva_facemark.h"
#include "m_cpp/m_vector_d.h"
#include "i_cpp/i_image.h"
#include "gr_cpp/gr_opengl.h"
#include "gr_cpp/gr_opengl_headers.h"
#include "gr_cpp/gr_primitive.h"
#include "gr_cpp/gr_sprite.h"

#ifdef KJB_HAVE_GLUT
#include "gr_cpp/gr_glut.h"
#endif

#include "l_cpp/l_exception.h"

#include <boost/foreach.hpp>
#include <boost/lexical_cast.hpp>

using namespace kjb;
using namespace kjb::pt;
using namespace kjb::opengl;

const Scene_viewer::Render_style
    Scene_viewer::DEFAULT_GROUND_STYLE(Vector(1.0, 1.0, 1.0), 1.0);
const Scene_viewer::Render_style
    Scene_viewer::DEFAULT_TARGET_STYLE(Vector(1.0, 1.0, 0.0), 1.0);
const Scene_viewer::Render_style
    Scene_viewer::DEFAULT_DATA_STYLE(Vector(1.0, 1.0, 0.0), 1.0);
const Vector Scene_viewer::DEFAULT_TEXT_COLOR(0.8, 0.8, 0.8);

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

Scene_viewer::Scene_viewer
(
    const Scene& scene, 
    double width, 
    double height, 
    bool show_window 
) :
#ifdef KJB_HAVE_GLUT
    m_glwin(0),
#endif
    m_scene_p(&scene),
    m_facemark_data_p(0),
    m_flowsx_p(0),
    m_flowsy_p(0),
    m_frame(1),
    m_draw_images(false),
    m_draw_text(false),
    m_weigh_box_color(false),
    m_draw_solid(false),
    m_ground_style(DEFAULT_GROUND_STYLE),
    m_target_style(DEFAULT_TARGET_STYLE),
    m_data_style(DEFAULT_DATA_STYLE),
    m_text_color(DEFAULT_TEXT_COLOR)
{
    if(show_window)
    {
#ifdef KJB_HAVE_GLUT
        m_glwin = new Glut_window(width, height, "View scene");
#endif
    }
    init_gl();
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

void Scene_viewer::set_scene(const Scene& scene)
{
    m_scene_p = &scene;

    clear_target_styles();
    clear_data_box_styles();

    if(m_frame == 0 || m_frame > scene_length())
    {
        set_frame(1);
    }

    if(scene_length() != m_frames_fps.size())
    {
        clear_frame_images();
    }

    if(m_draw_labels) build_entity_map();
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

void Scene_viewer::resize(size_t width, size_t height)
{
#ifdef KJB_HAVE_GLUT
    if(m_glwin)
    {
        m_glwin->set_size(width, height);
    }
#endif
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

double Scene_viewer::width() const
{
    return get_viewport_width();
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

double Scene_viewer::height() const
{
    return get_viewport_height();
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

void Scene_viewer::set_frame(size_t f)
{
    IFT(f >= 1 && f <= scene_length(),
        Illegal_argument, "Cannot set frame: value outside of range.");

    m_frame = f;
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

void Scene_viewer::set_overhead_view()
{
    m_alt_camera = m_scene_p->camera;
    m_alt_camera->set_camera_centre(Vector(0.0, 15.0, -10.0));
    m_alt_camera->set_roll(0.0);
    m_alt_camera->set_yaw(0.0);
    m_alt_camera->set_pitch(M_PI/2);

    draw_images(false);
    draw_ground(true);
    draw_cylinders(false);
    draw_bodies(false);
    draw_heads(false);
    draw_faces(false);
    draw_bottoms(true);
    draw_trails(true);
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

void Scene_viewer::write_images(const std::vector<std::string>& out_fps)
{
    for(size_t i = 0; i < scene_length(); ++i)
    {
        render_scene();
        Image img = get_framebuffer_as_image();
        img.write(out_fps[i]);
        advance_frame();
    }
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

void Scene_viewer::draw_labels(bool dl)
{
    m_draw_labels = dl; 
    if(m_draw_labels) 
    {
#ifdef KJB_HAVE_GLUT
        if(!m_glwin)
        {
            std::cerr << "WARNING: GLUT windown is not initialized\n";
        }
        build_entity_map();
#else
        std::cerr << "WARNING: Labels can not be rendered in offscreen\n";
#endif
    }
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

void Scene_viewer::draw_3d_mode()
{
    m_draw_ground = true;
    m_draw_cylinders = false;
    m_draw_bodies = true;
    m_draw_objects = true;
    m_draw_heads = true;
    m_draw_faces = true;
    m_draw_bottoms = true;
    m_draw_trails = true;
    m_draw_full_boxes = false;
    m_draw_body_boxes = false;
    m_draw_face_boxes = false;
    m_draw_face_features = false;
    m_draw_model_vectors = false;
    m_draw_face_vectors = false;
    m_draw_data_boxes = false;
    m_draw_facemarks = false;
    m_draw_flow_vectors = false;
    m_draw_flow_face_vectors = false;
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

void Scene_viewer::draw_2d_mode()
{
    m_draw_ground = false;
    m_draw_cylinders = false;
    m_draw_bodies = false;
    m_draw_heads = false;
    m_draw_faces = false;
    m_draw_bottoms = false;
    m_draw_trails = false;
    m_draw_full_boxes = false;
    m_draw_body_boxes = true;
    m_draw_face_boxes = true;
    m_draw_face_features = true;
    m_draw_model_vectors = true;
    m_draw_face_vectors = true;
    m_draw_data_boxes = true;
    m_draw_facemarks = true;
    m_draw_flow_vectors = true;
    m_draw_flow_face_vectors = true;
}


/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

void Scene_viewer::advance_frame(size_t df)
{
    m_frame += df;

    if(m_frame > scene_length())
    {
        size_t ndf = m_frame - scene_length() - 1;
        m_frame = 1;
        advance_frame(ndf);
    }
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

void Scene_viewer::rewind_frame(size_t df)
{
    if(df >= m_frame)
    {
        size_t ndf = df - m_frame;
        m_frame = scene_length();
        rewind_frame(ndf);
    }
    else
    {
        m_frame -= df;
    }
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

void Scene_viewer::set_target_style
(
    const Target& target,
    const Vector& color,
    double line_width,
    bool only
)
{
    if(only)
    {
        clear_target_styles();
    }

    Render_style& rs = m_target_styles.insert(
            std::make_pair(&target, m_target_style)).first->second;

    rs.color = color;
    rs.line_width = line_width;
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

void Scene_viewer::set_target_data_style
(
    const Target& target,
    const Vector& color,
    double line_width,
    bool only
)
{
    if(only)
    {
        clear_target_styles();
    }

    // set box styles
    BOOST_FOREACH(const Target::value_type& pr, target)
    {
        set_data_box_style(*pr.second, color, line_width, false);
    }

    // set facemark styles
    size_t sf = target.get_start_time();
    size_t ef = target.get_end_time();
    for(size_t t = sf; t <= ef; ++t)
    {
        const Deva_facemark* fm_p
            = target.face_trajectory()[t - 1]->value.facemark;
        if(fm_p != 0)
        {
            set_facemark_style(*fm_p, color, line_width, false);
        }
    }
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

void Scene_viewer::set_data_box_style
(
    const Detection_box& dbox,
    const Vector& color,
    double line_width,
    bool only
)
{
    if(only)
    {
        clear_data_box_styles();
    }

    Render_style& rs = m_data_box_styles.insert(
            std::make_pair(&dbox, m_data_style)).first->second;

    rs.color = color;
    rs.line_width = line_width;
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

void Scene_viewer::set_facemark_style
(
    const Deva_facemark& fm,
    const Vector& color,
    double line_width,
    bool only
)
{
    if(only)
    {
        clear_facemark_styles();
    }

    Render_style& rs = m_facemark_styles.insert(
            std::make_pair(&fm, m_data_style)).first->second;

    rs.color = color;
    rs.line_width = line_width;
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

void Scene_viewer::build_entity_map()
{
    m_entity_map.clear();
    //  persons
    const Entity_type person_et = get_entity_type("person");
    const Entity_type object_et = get_entity_type("object");

    size_t idx = 1;
    BOOST_FOREACH(const Target& tg, m_scene_p->association)
    {
        Entity_id eid(person_et, idx++);
        m_entity_map.insert(std::make_pair(&tg, eid));
    }

    BOOST_FOREACH(const Target& tg, m_scene_p->objects)
    {
        Entity_id eid(object_et, idx++);
        m_entity_map.insert(std::make_pair(&tg, eid));
    }
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

void Scene_viewer::prepare_for_rendering_2d() const
{
    double w = get_viewport_width();
    double h = get_viewport_height();;

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-w / 2.0, w / 2.0, -h / 2.0, h / 2.0, -1.0, 1.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

void Scene_viewer::init_gl()
{
    // general stuff
    glClearColor(0.0, 0.0, 0.0, 0.0);
    glEnable(GL_DEPTH_TEST);
    //glEnable(GL_CULL_FACE);
    init_lighting();

    // render 3D stuff by default
    draw_3d_mode();

    // display callback
#ifdef KJB_HAVE_GLUT
    if(m_glwin)
    {
        m_glwin->set_reshape_callback(boost::bind(
                    &Scene_viewer::reshape_scene, this, _1, _2));
        m_glwin->set_display_callback(boost::bind(
                    &Scene_viewer::render_scene, this));
    }
#endif
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

void Scene_viewer::init_lighting() const
{
    // enable depth test
    glEnable(GL_DEPTH_TEST);

    // ambient light (commented out lines are the GL default already)
    //GLfloat lmodel_ambient[] = { 0.2, 0.2, 0.2, 1.0 };
    //glLightModelfv(GL_LIGHT_MODEL_AMBIENT, lmodel_ambient);
    glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);

    // point light (commented out lines are the GL default already)
    GLfloat light_position[] = { 0.0, 1.0, 1.0, 0.0 };
    //GLfloat am_light_color[] = { 0.0, 0.0, 0.0, 1.0 };
    //GLfloat df_light_color[] = { 1.0, 1.0, 1.0, 1.0 };
    GLfloat sp_light_color[] = { 0.5, 0.5, 0.5, 1.0 };
    glLightfv(GL_LIGHT0, GL_POSITION, light_position);
    //glLightfv(GL_LIGHT0, GL_AMBIENT, am_light_color);
    //glLightfv(GL_LIGHT0, GL_DIFFUSE, df_light_color);
    glLightfv(GL_LIGHT0, GL_SPECULAR, sp_light_color);

    // material shihiness and specularity is the same for all
    // (commented out lines are the GL default already)
    GLfloat sp_material_color[] = { 0.2, 0.2, 0.2, 1.0 };
    //GLfloat em_material_color[] = { 0.0, 0.0, 0.0, 1.0 };
    GLfloat material_shininess[] = { 32 };
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, sp_material_color);
    //glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, em_material_color);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, material_shininess);

    // lighting modes
    glEnable(GL_LIGHT0);
    glEnable(GL_NORMALIZE);
    glShadeModel(GL_SMOOTH);
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

void Scene_viewer::reshape_scene(int w, int h) const
{
    glViewport(0, 0, w, h);
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

void Scene_viewer::render_scene() const
{
    // clear stuff
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();
    glDisable(GL_LIGHTING);

    if(m_draw_images)
    {
        IFT(m_frames_fps.size() != 0, Runtime_error,
            "Cannot display frames; images not specified.");

        Image img(m_frames_fps[m_frame - 1]);
        Sprite sprite(img, 0);
        sprite.set_position(width()/2, height()/2);
        sprite.center_origin();
        sprite.render();
    }
    
    // render 3D stuff
    if(m_alt_camera) m_alt_camera->prepare_for_rendering(false);
    else m_scene_p->camera.prepare_for_rendering(false);
    if(m_draw_solid) glEnable(GL_LIGHTING);

    render_ground();
    render_cylinders();
    render_heads();
    render_bottoms();
    render_objects();

    glDisable(GL_LIGHTING);
    render_trails();

    // render 2D stuff
    prepare_for_rendering_2d();
    if(!m_alt_camera)
    {
        // these only make sense when using scene camera
        render_model_boxes();
        render_face_boxes();
        render_model_vectors();
        render_data_boxes();
        render_facemarks();
        render_flow_vectors();
    }

    // render text
    render_text();

    // errors?
    GL_ETX();

#ifdef KJB_HAVE_GLUT
    if(m_glwin) glutSwapBuffers();
#endif
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

void Scene_viewer::render_ground() const
{
    if(!m_draw_ground) return;

    // save state
    //glPushAttrib(GL_ALL_ATTRIB_BITS);

    if(m_draw_solid)
    {
        const Vector& gcol = m_ground_style.color;
        GLfloat r = gcol[0];
        GLfloat g = gcol[1];
        GLfloat b = gcol[2];
        GLfloat diffuse_col[] = { r, g, b, 1.0 };
        glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, diffuse_col);
        glBegin(GL_POLYGON);
            glVertex(Vector(-10, -0.01, 0));
            glVertex(Vector(10, -0.01, 0));
            glVertex(Vector(10, -0.01, -100));
            glVertex(Vector(-10, -0.01, -100));
        glEnd();
    }
    else
    {
        // set up blending
        //glEnable(GL_BLEND);
        //glShadeModel(GL_FLAT);
        //glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        // draw ground
        glColor(m_ground_style.color);
        glLineWidth(m_ground_style.line_width);
        render_xz_plane_grid(-100, 100, -100, 0, 2);
        //glDisable(GL_BLEND);
    }

    // reset state
    //glPopAttrib();
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

void Scene_viewer::render_cylinders() const
{
    if(!m_draw_cylinders && !m_draw_bodies) return;

    BOOST_FOREACH(const Target& tg, m_scene_p->association)
    {
        //glColor(get_target_color(tg));
        //glLineWidth(get_target_lwidth(tg));
        set_current_style(tg);

        const Trajectory& tj = tg.trajectory();
        double wt = tj.width;
        double gt = tj.girth;
        double ht = tj.height;

        if(tj[m_frame - 1])
        {
            const Complete_state& cs = tj[m_frame - 1]->value;
            const Vector3& pos = cs.position;
            double body_dir = cs.body_dir;

            glPushMatrix();
                glTranslate(pos);
                glRotated(180*body_dir/M_PI, 0.0, 1.0, 0.0);
                glScaled(1.0, 1.0, gt / wt);
                glRotated(-90.0, 1.0, 0.0, 0.0);
                Head head(cs, ht, wt, gt);

                // draw full cylinder
                if(m_draw_cylinders)
                {
                    Cylinder cyl(wt / 2.0, wt / 2.0, ht, 30, 30);
                    if(m_draw_solid) cyl.solid_render();
                    else cyl.wire_render();
                }

                // draw body cylinder
                if(m_draw_bodies)
                {
                    Cylinder cyl(wt / 2.0, wt / 2.0, ht - head.height, 30, 30);
                    if(m_draw_solid) cyl.solid_render();
                    else cyl.wire_render();
                }

            glPopMatrix();

            // draw arrow to distinguish front
            double arrw = 0.05;
            Vector c(pos[0], ht - head.height - arrw/2, pos[2]);
            Vector ae = ellipse_point(cs, ht - head.height - arrw/2, wt, gt, 0.0);
            Vector as = ae;
            ae -= c;
            ae = 2*ae;
            Arrow3d arrow(as, ae, arrw);
            if(m_draw_solid) arrow.solid_render();
            else arrow.wire_render();
        }
    }
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

void Scene_viewer::render_heads() const
{
    if(!m_draw_heads && !m_draw_faces) return;

    BOOST_FOREACH(const Target& tg, m_scene_p->association)
    {
        //glColor(get_target_color(tg));
        //glLineWidth(get_target_lwidth(tg));
        set_current_style(tg);

        const Trajectory& tj = tg.trajectory();
        double wt = tj.width;
        double gt = tj.girth;
        double ht = tj.height;

        if(tj[m_frame - 1])
        {
            const Complete_state& cs = tj[m_frame - 1]->value;
            Head head(cs, ht, wt, gt);

            // draw head
            if(m_draw_heads)
            {
                Sphere sph(1.0, 15, 15);
                glPushMatrix();
                    glTranslate(head.center);
                    glRotate(head.orientation);
                    //glRotate(Quaternion(head.orientation));
                    glScaled(head.width/2, head.height/2, head.girth/2);
                    if(m_draw_solid) sph.solid_render();
                    else sph.wire_render();
                glPopMatrix();
            }

            // arrow
            double arrw = 0.025;
            Vector as = head_to_world(Vector(0.0, 1.0, 0.0), head, true);
            Vector ae = head_to_world(Vector(0.0, 1.0, 2.0), head, true);
            ae -= as;
            Arrow3d arrow(as, ae, arrw);
            if(m_draw_solid) arrow.solid_render();
            else arrow.wire_render();

            if(cs.attn != 0)
            {
                //Sphere asp(arrw * 2, 15, 15);
                //glPushMatrix();
                //    glTranslate(as + ae);
                //    set_current_style(*cs.attn);
                //    if(m_draw_solid) asp.solid_render();
                //    else asp.wire_render();
                //glPopMatrix();
                GLboolean lgton;
                glGetBooleanv(GL_LIGHTING, &lgton);
                glDisable(GL_LIGHTING);

                Vector3 ept;
                if(cs.attn->trajectory().id.type == PERSON_ENTITY)
                {
                    ASSERT(cs.attn->trajectory()[m_frame - 1]);
                    ept = cs.attn->trajectory()[m_frame - 1]->value.position;
                    ept[1] = cs.attn->trajectory().height;
                }
                else
                {
                    ASSERT(cs.attn->trajectory()[0]);
                    ept = cs.attn->trajectory()[0]->value.position;
                }

                glColor(get_target_color(tg));
                glBegin(GL_LINES);
                    glVertex(as);
                    glVertex(ept);
                glEnd();

                if(lgton) glEnable(GL_LIGHTING);
            }

            // draw face
            if(m_draw_faces)
            {
                // no light for face features
                GLboolean lgton;
                glGetBooleanv(GL_LIGHTING, &lgton);
                glDisable(GL_LIGHTING);

                Ellipse ell(head.height/20, head.height/20);
                glColor3d(1.0, 1.0, 1.0);
                if(m_draw_solid)
                {
                    GLfloat diffuse_col[] = {1.0f, 1.0f, 1.0f, 1.0 };
                    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, diffuse_col);
                }
                Vector_vec ffts = face_features(cs, ht, wt, gt);
                for(size_t i = 0; i < ffts.size(); i++)
                {
                    glPushMatrix();
                        glTranslate(ffts[i]);
                        glRotate(head.orientation);
                        //glRotate(Quaternion(head.orientation));
                        glTranslated(0.0, 0.0, head.height/25);
                        ell.solid_render();
                    glPopMatrix();
                }

                if(lgton) glEnable(GL_LIGHTING);
            }
        }
    }
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

void Scene_viewer::render_bottoms() const
{
    if(!m_draw_bottoms) return;

    BOOST_FOREACH(const Target& tg, m_scene_p->association)
    {
        //glColor(get_target_color(tg));
        //glLineWidth(get_target_lwidth(tg));
        set_current_style(tg);

        const Trajectory& tj = tg.trajectory();
        double wt = tj.width;
        double gt = tj.girth;
        if(tj[m_frame - 1])
        {
            const Vector3& pos = tj[m_frame - 1]->value.position;
            double body_dir = tj[m_frame - 1]->value.body_dir;

            glPushMatrix();
                Ellipse ellipse(wt / 2.0, gt / 2.0);
                glTranslate(pos);
                glRotated(180*body_dir/M_PI, 0.0, 1.0, 0.0);
                glRotated(-90.0, 1.0, 0.0, 0.0);

                ellipse.solid_render();
            glPopMatrix();
        }
    }
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

void Scene_viewer::render_objects() const
{
    if(!m_draw_objects) return;

    BOOST_FOREACH(const Target& tg, m_scene_p->objects)
    {
        set_current_style(tg);
        const Vector3& pos = tg.trajectory().front()->value.position;

        Sphere pot(0.1);
        //Teapot pot;
        glPushMatrix();
            glTranslate(pos);
            //glScaled(0.2, 0.2, 0.2);
            if(m_draw_solid) pot.solid_render();
            else pot.wire_render();
        glPopMatrix();
    }
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

void Scene_viewer::render_trails() const
{
    if(!m_draw_trails) return;

    BOOST_FOREACH(const Target& tg, m_scene_p->association)
    {
        glColor(get_target_color(tg));
        glLineWidth(get_target_lwidth(tg));

        const size_t start_frame = tg.get_start_time();
        const size_t end_frame = tg.get_end_time();
        const Trajectory& tj = tg.trajectory();
        if(m_frame >= start_frame && m_frame <= end_frame)
        {
            glBegin(GL_LINE_STRIP);
            for(size_t t = start_frame; t <= std::min(m_frame, end_frame); t++)
            {
                ASSERT(tj[t - 1]);
                glVertex(tj[t - 1]->value.position);
            }
            glEnd();
        }
    }
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

void Scene_viewer::render_model_boxes() const
{
    if(!m_draw_full_boxes && !m_draw_body_boxes) return;

    BOOST_FOREACH(const Target& tg, m_scene_p->association)
    {
        glColor(get_target_color(tg));
        glLineWidth(get_target_lwidth(tg));

        const Body_2d_trajectory& tj = tg.body_trajectory();
        if(tj[m_frame - 1])
        {
            const Bbox& fbox = tj[m_frame - 1]->value.full_bbox;
            const Bbox& bbox = tj[m_frame - 1]->value.body_bbox;

            if(m_draw_full_boxes) fbox.wire_render();
            if(m_draw_body_boxes) bbox.wire_render();

#ifdef KJB_HAVE_GLUT
            if(m_draw_labels)
            {
                const double padding = 10;
                const Vector& center = fbox.get_center();
                double top = fbox.get_top();
                double x = center[0];
                double y = center[1] + y;
                Entity_id entity_id = m_entity_map.at(&tg); 
                std::string person_label = 
                                get_entity_type_name(entity_id.type) + "-" + 
                                boost::lexical_cast<std::string>(
                                        entity_id.index);
                if(m_glwin) bitmap_string(person_label, x, y);
            }
#endif
        }
    }
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

void Scene_viewer::render_face_boxes() const
{
    if(!m_draw_face_boxes && !m_draw_face_features) return;

    BOOST_FOREACH(const Target& tg, m_scene_p->association)
    {
        glColor(get_target_color(tg));
        glLineWidth(get_target_lwidth(tg));

        const Face_2d_trajectory& tj = tg.face_trajectory();
        if(tj[m_frame - 1])
        {
            const Face_2d& f2d = tj[m_frame - 1]->value;

            if(m_draw_face_boxes)
            {
                f2d.bbox.wire_render();
            }

            if(m_draw_face_features)
            {
                double fsz = f2d.bbox.get_height() / 25;
                Bbox bx(Vector(0.0, 0.0), fsz, fsz);

                if(!f2d.left_eye.empty())
                {
                    bx.set_center(f2d.left_eye);
                    bx.wire_render();
                }

                if(!f2d.right_eye.empty())
                {
                    bx.set_center(f2d.right_eye);
                    bx.wire_render();
                }

                if(!f2d.nose.empty())
                {
                    bx.set_center(f2d.nose);
                    bx.wire_render();
                }

                if(!f2d.left_mouth.empty())
                {
                    bx.set_center(f2d.left_mouth);
                    bx.wire_render();
                }

                if(!f2d.right_mouth.empty())
                {
                    bx.set_center(f2d.right_mouth);
                    bx.wire_render();
                }
            }
        }
    }
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

void Scene_viewer::render_model_vectors() const
{
    if(!m_draw_model_vectors && !m_draw_face_vectors) return;

    BOOST_FOREACH(const Target& tg, m_scene_p->association)
    {
        glColor(get_target_color(tg));
        glLineWidth(get_target_lwidth(tg));

        const Body_2d_trajectory& tj = tg.body_trajectory();
        const Face_2d_trajectory& ftj = tg.face_trajectory();
        if(tj[m_frame - 1] && m_frame != tg.get_end_time())
        {
            const size_t t = m_frame - 1;

            if(m_draw_model_vectors)
            {
                const Vector& bc = tj[t]->value.body_bbox.get_center();
                const Vector& cbc = tj[t]->value.body_bbox.get_top_center();
                const Vector& nbc = tj[t + 1]->value.body_bbox.get_top_center();
                Vector vel = 10*(nbc - cbc);

                glPushMatrix();
                    glTranslated(0.0, 0.0, 0.2);
                    render_arrow(bc, bc + vel);
                glPopMatrix();
            }

            if(m_draw_face_vectors)
            {
                const Vector& bc = ftj[t]->value.bbox.get_center();
                Vector vel = 10*ftj[t]->value.model_dir;

                glPushMatrix();
                    glTranslated(0.0, 0.0, 0.2);
                    render_arrow(bc, bc + vel);
                glPopMatrix();
            }
        }
    }
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

void Scene_viewer::render_data_boxes() const
{
    if(!m_draw_data_boxes) return;

    const Ascn& w = m_scene_p->association;
    BOOST_FOREACH(const Detection_box& dbox, w.get_data()[m_frame - 1])
    {
        double bcw = m_weigh_box_color ? 1.0 - dbox.prob_noise : 1.0;

        glColor(bcw * get_data_box_color(dbox));
        glLineWidth(get_data_box_lwidth(dbox));

        const Bbox& bbox = dbox.bbox;

        glPushMatrix();
        glTranslated(0.0, 0.0, 0.1);
        bbox.wire_render();
        glPopMatrix();
    }
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

void Scene_viewer::render_facemarks() const
{
    if(!m_draw_facemarks) return;

    IFT(m_facemark_data_p != 0, Runtime_error,
        "Cannot render facemarks; they were never specified.");

    BOOST_FOREACH(const Deva_facemark& fmark, (*m_facemark_data_p)[m_frame - 1])
    {
        glColor(get_facemark_color(fmark));
        glLineWidth(get_facemark_lwidth(fmark));

        if(!fmark.left_eye_mark().empty())
        {
            render_x(fmark.left_eye_mark(), 6);
        }

        if(!fmark.right_eye_mark().empty())
        {
            render_x(fmark.right_eye_mark(), 6);
        }

        if(!fmark.nose_mark().empty())
        {
            render_x(fmark.nose_mark(), 6);
        }

        if(!fmark.left_mouth_mark().empty())
        {
            render_x(fmark.left_mouth_mark(), 6);
        }

        if(!fmark.right_mouth_mark().empty())
        {
            render_x(fmark.right_mouth_mark(), 6);
        }
    }
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

void Scene_viewer::render_flow_vectors() const
{
    if(!m_draw_flow_vectors && !m_draw_flow_face_vectors) return;

    IFT(m_flowsx_p != 0 && m_flowsy_p != 0, Runtime_error,
        "Cannot render flow vectors; no features specified.");

    glColor(m_data_style.color);
    glLineWidth(m_data_style.line_width);
    BOOST_FOREACH(const Target& tg, m_scene_p->association)
    {
        const Body_2d_trajectory& btj = tg.body_trajectory();
        const Face_2d_trajectory& ftj = tg.face_trajectory();
        if(btj[m_frame - 1] && m_frame != tg.get_end_time())
        {
            // flow features
            const Integral_flow& flow_x = (*m_flowsx_p)[m_frame - 1];
            const Integral_flow& flow_y = (*m_flowsy_p)[m_frame - 1];

            if(m_draw_flow_vectors)
            {
                // box and visibility
                const Bbox& bbox = btj[m_frame - 1]->value.body_bbox;
                const Visibility& vis = btj[m_frame - 1]->value.visibility;
                render_flow_arrow(bbox, vis, flow_x, flow_y);
            }

            if(m_draw_flow_face_vectors)
            {
                // box and visibility
                const Bbox& bbox = ftj[m_frame - 1]->value.bbox;
                const Visibility& vis = ftj[m_frame - 1]->value.visibility;
                render_flow_arrow(bbox, vis, flow_x, flow_y);
            }
        }
    }
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

void Scene_viewer::render_text() const
{
    if(!m_draw_text) return;

    const size_t num_lines = m_text_lines.size();
    const size_t char_width = 8;
    const size_t char_height = 13;
    const size_t xsep = char_width;
    const size_t ysep = 4;

    double line_height = char_height + ysep;

    // get text width
    double text_width = 0.0;
    for(size_t i = 0; i < num_lines; i++)
    {
        double line_width = char_width * m_text_lines[i].size();
        text_width = std::max(text_width, line_width);
    }

    text_width += xsep;

    double x = width()/2.0 - text_width;
    double y = height()/2.0 - line_height;
    for(size_t i = 0; i < num_lines; i++)
    {
        glColor(get_text_line_color(m_text_lines[i]));
        bitmap_string(m_text_lines[i], x, y);
        y -= line_height;
    }
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

void Scene_viewer::render_flow_arrow
(
    const Bbox& box,
    const Visibility& vis,
    const Integral_flow& fx,
    const Integral_flow& fy
) const
{
    // if box is invisible, skip
    if(vis.visible == 0.0) return;

    // average flow
    Vector avg_flow = average_box_flow(fx, fy, box, vis);

    // arrow
    Vector src = box.get_centre();
    unstandardize(src, width(), height());
    Vector dest = src + 10*avg_flow;
    standardize(src, width(), height());
    standardize(dest, width(), height());
    render_arrow(src, dest);
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

void Scene_viewer::set_current_style(const Target& tg) const
{
    if(m_draw_solid)
    {
        const Vector& tgcol = get_target_color(tg);
        GLfloat r = tgcol[0];
        GLfloat g = tgcol[1];
        GLfloat b = tgcol[2];
        GLfloat diffuse_col[] = { r, g, b, 1.0 };
        glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, diffuse_col);
    }
    else
    {
        glColor(get_target_color(tg));
        glLineWidth(get_target_lwidth(tg));
    }
}


/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

const Vector& Scene_viewer::get_target_color(const Target& target) const
{
    typedef std::map<const Target*, Render_style> Ts_map;

    Ts_map::const_iterator tsp_p = m_target_styles.find(&target);
    if(tsp_p != m_target_styles.end())
    {
        return tsp_p->second.color;
    }

    return m_target_style.color;
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

double Scene_viewer::get_target_lwidth(const Target& target) const
{
    typedef std::map<const Target*, Render_style> Ts_map;

    Ts_map::const_iterator tsp_p = m_target_styles.find(&target);
    if(tsp_p != m_target_styles.end())
    {
        return tsp_p->second.line_width;
    }

    return m_target_style.line_width;
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

const Vector& Scene_viewer::get_data_box_color(const Detection_box& dbox) const
{
    typedef std::map<const Detection_box*, Render_style> Ds_map;

    Ds_map::const_iterator dsp_p = m_data_box_styles.find(&dbox);
    if(dsp_p != m_data_box_styles.end())
    {
        return dsp_p->second.color;
    }

    return m_data_style.color;
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

double Scene_viewer::get_data_box_lwidth(const Detection_box& dbox) const
{
    typedef std::map<const Detection_box*, Render_style> Ds_map;

    Ds_map::const_iterator dsp_p = m_data_box_styles.find(&dbox);
    if(dsp_p != m_data_box_styles.end())
    {
        return dsp_p->second.line_width;
    }

    return m_data_style.line_width;
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

const Vector& Scene_viewer::get_facemark_color(const Deva_facemark& fmark) const
{
    typedef std::map<const Deva_facemark*, Render_style> Fm_map;

    Fm_map::const_iterator fsp_p = m_facemark_styles.find(&fmark);
    if(fsp_p != m_facemark_styles.end())
    {
        return fsp_p->second.color;
    }

    return m_data_style.color;
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

double Scene_viewer::get_facemark_lwidth(const Deva_facemark& fmark) const
{
    typedef std::map<const Deva_facemark*, Render_style> Fm_map;

    Fm_map::const_iterator fsp_p = m_facemark_styles.find(&fmark);
    if(fsp_p != m_facemark_styles.end())
    {
        return fsp_p->second.line_width;
    }

    return m_data_style.line_width;
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

Vector Scene_viewer::get_text_line_color(const std::string& /*str*/) const
{
    return m_text_color;
}

#endif // KJB_HAVE_OPENGL
