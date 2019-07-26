/**
 * This work is licensed under a Creative Commons
 * Attribution-Noncommercial-Share Alike 3.0 United States License.
 *
 *    http://creativecommons.org/licenses/by-nc-sa/3.0/us/
 *
 * You are free:
 *
 *    to Share - to copy, distribute, display, and perform the work
 *    to Remix - to make derivative works
 *
 * Under the following conditions:
 *
 *    Attribution. You must attribute the work in the manner specified by the
 *    author or licensor (but not in any way that suggests that they endorse you
 *    or your use of the work).
 *
 *    Noncommercial. You may not use this work for commercial purposes.
 *
 *    Share Alike. If you alter, transform, or build upon this work, you may
 *    distribute the resulting work only under the same or similar license to
 *    this one.
 *
 * For any reuse or distribution, you must make clear to others the license
 * terms of this work. The best way to do this is with a link to this web page.
 *
 * Any of the above conditions can be waived if you get permission from the
 * copyright holder.
 *
 * Apart from the remix rights granted under this license, nothing in this
 * license impairs or restricts the author's moral rights.
 */

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
|     Joseph Schlecht, Luca Del Pero, Kyle Simek
|
* =========================================================================== */

/**
 * @file
 *
 * @author Joseph Schlecht, Luca Del Pero, Kyle Simek
 *
 * @brief Abstract class to render this object with GL.
 */


#ifndef KJB_RENDERABLE_H
#define KJB_RENDERABLE_H

#include <m_cpp/m_matrix.h>
#include <l_cpp/l_exception.h>

namespace kjb {

class Base_gl_interface;

/**
 * @class Renderable
 *
 * @brief Abstract class to render this object with GL.
 */
class Renderable
{
public:
    /** @brief Deletes this Renderable. */
    virtual ~Renderable() { }


    /** @brief Renders this object with GL. */
    virtual void render() const = 0;
};


/**
 * @class Wire_renderable
 *
 * @brief Abstract class to render this object with GL as a wire-frame.
 */
class Wire_renderable
{
public:
    /** @brief Deletes this Wire_renderable. */
    virtual ~Wire_renderable() { }


    /** @brief Renders this object with GL as a wire-frame. */
    virtual void wire_render() const = 0;
};

/**
 * @class Solid_renderable
 *
 * @brief Abstract class to render this object with GL
 */
class Solid_renderable
{
public:

    /** @brief Deletes this Wire_renderable. */
    virtual ~Solid_renderable() { }


    /** @brief Renders this object with GL as a wire-frame. */
    virtual void solid_render() const = 0;
};


/**
 * @class Wire_occlude_renderable
 *
 * @brief Abstract class to render this object with GL as an occluded
 * wire-frame into the depth buffer, to hide unseen lines.
 */
class Wire_occlude_renderable
{
public:

    /** @brief Deletes this Wire_occlude_renderable. */
    virtual ~Wire_occlude_renderable() { }


    /**
     * @brief Renders this object with GL as a wireframe
     * occluding hidden lines
     */
    virtual void wire_occlude_render() const = 0;
};


/**
 * @class Abstract
 *
 * @brief Abstract class to render this object
 */
class Abstract_renderable : public Renderable,
                            public Solid_renderable,
                            public Wire_renderable,
                            public Wire_occlude_renderable
{
public:

    /** @brief Deletes this Solid_renderable. */
    virtual ~Abstract_renderable() { }

    virtual void render() const
    {
        solid_render();
    }

    /** 
     * @brief   returns the rendering framework used to render. The only one
     *          implemented up to now is OpenGL
     */
    static double get_rendering_framework(){ return _rendering_framework; }

    /**
     * @brief   sets the rendering framework used to render. The only one 
     *          implemented up to now is OpenGL
     */
    static void set_rendering_framework(unsigned int irf)
    {
        _rendering_framework = irf;
    }

    /**
     * @brief  Renders this object with GL as an occluded wire-frame
     *         into the depth buffer, to hide unseen lines.
     */
    virtual void render_occluded_wireframe() const
    {
        wire_occlude_render();
        wire_render();
    }

    /**
     * @brief  Renders the silhouette of this object. The most basic implementation is
     *         to render the occluded wireframe
     */
    virtual void silhouette_render(const kjb::Base_gl_interface &, double width) const
    {
        width = width + 0.0; //This is to avoid a warning
        render_occluded_wireframe();
    }

protected:
    enum Rendering_frameworks
    {
        RI_OPENGL
    };

    /**
     * The rendering framework used to render. The only one implemented
     * up to now is OpenGL
     */
    static unsigned int _rendering_framework;
};

/**
 * @class Generic_renderable
 *
 * This class is for objects which have all forms of renderability,
 * and provides some default implementation for wire and occluded wire rendering.
 *
 * The inherited class need only implement solid_render() -- wire rendering and
 * occluded wire rendering are implemented using solid_render().  The provided
 * method for occluded wire rendering doesn't draw interior edges, only sillhouette
 * edges are drawn.  Inherited classes are free to uoverride this functionality if desired.
 *
 * Inherited classes only need to implement solid_render() to get all three types of
 * rendering.  Optionally, the inherited class may choose between the two methods for
 * occluded edge rendering offered by Generic_renderable, to balance speed and quality,
 * or implement its own method.
 *
 * @author Kyle Simek
 *
 */
class Generic_renderable :
    public kjb::Abstract_renderable
{
public:

    /** @brief Render wire mesh */
    virtual void wire_render() const;

    /** @brief Render silhouette edges */
    virtual void wire_occlude_render() const
    {
        _opengl_offset_edge(0, 0);
    }
    /** @brief Render silhouette edges */
    virtual void wire_occlude_render(double offset_factor, double offset_units) const
    {
        _opengl_offset_edge(offset_factor, offset_units);
    }

    /** @brief Render solid object */
    virtual void solid_render() const = 0;


    /**
     * Set the "transparent" color.  Pixels lying in the
     * interior of the object will be rendered this color
     *
     * Default is white.
     *
     * @kludge Added this during CVPR 2010 to get white-on-black edges for GPU-based chamfer likelihood.  Needs a better design decision. --Kyle
     */
    static void set_background_color(const Vector& rgba)
    {
        background_color = rgba;
    }

    /**
     * Set the edge color.  Pixels lying on the border of the object's silhouette
     * will be this color.
     *
     * Default is black.
     *
     * @kludge Added this during CVPR 2010 to get white-on-black edges for GPU-based chamfer likelihood.  Needs a better design decision. --Kyle
     */
    static void set_foreground_color(const Vector& rgba)
    {
        foreground_color = rgba;
    }

protected:
    /** @brief General edge-rendering algorithm using stencil buffer */
    void _opengl_stencil_edge(bool hollow = true) const;

    /** @brief General edge_rendering algorithm using polygon offset */
    void _opengl_offset_edge(double offset_factor = 0, double offset_units = 0) const;

private:
    static Vector background_color;
    static Vector foreground_color;
};



/**
 * @class Generic_renderer
 *
 * Allows us to pass a rendering technique to a generic rendering function.
 *
 * @author Kyle Simek
 *
 */
class Generic_renderer : public Renderable
{
public:
    Generic_renderer() {}

    Generic_renderer(const Generic_renderable* model) :
        m_model(model)
    {
        IFT(m_model != NULL, Illegal_argument, "Given model is NULL.");
    }

    virtual void render() const
    {
        IFT(m_model != NULL, KJB_error, "Model not set.");
        render(*m_model);
    }

    virtual void render(const Generic_renderable&) const = 0;

    virtual void operator()(const Generic_renderable& model) const
    {
        render(model);
    }
protected:
    const Generic_renderable* m_model;
};

/**
 * @class Renderer
 *
 * Allows us to pass the default rendering technique to a generic rendering function.
 *
 * @author Kyle Simek
 *
 */
class Renderer : public Generic_renderer
{
public:
    /**
     * Note: caller is responsible for deleting model
     */
    Renderer() : Generic_renderer() {}

    Renderer(const Generic_renderable* model) :
        Generic_renderer(model) {}

    virtual void render(const Generic_renderable& model) const
    {
        model.render();
    }
};

/**
 * @class Wire_renderer
 *
 * Allows us to pass the wire rendering technique to a generic rendering function.
 *
 * @author Kyle Simek
 * */
class Wire_renderer : public Generic_renderer
{
public:
    /**
     * Note: caller is responsible for deleting model
     */
    Wire_renderer() : Generic_renderer() {}

    Wire_renderer(const Generic_renderable* model) :
        Generic_renderer(model) {}

    virtual void render(const Generic_renderable& model) const
    {
        model.wire_render();
    }
};

/**
 * @class Wire_occlude_renderer
 *
 * Allows us to pass the occluded wire rendering technique to a generic rendering function.
 *
 * @author Kyle Simek
 * */
class Wire_occlude_renderer : public Generic_renderer
{
public:
    /**
     * Note: caller is responsible for deleting model
     *
     * @param   offset_factor   See glPolygonOffset. Negative values give less
     *                          stitching, but too extreme can create other
     *                          rendering artifacts. Default = 0; means use
     *                          algorithm default.
     *
     * @param   offset_units    See glPolygonOffset. Negative values  give less
     *                          stitching, but too extreme can create other
     *                          rendering artifacts. Default = 0; means use
     *                          algorithm default.
     */
    Wire_occlude_renderer(float offset_factor = 0, float offset_units = 0) :
        Generic_renderer(),
        m_offset_factor(offset_factor),
        m_offset_units(offset_units)
    {}

    /**
     * Note: caller is responsible for deleting model
     *
     * @param   offset_factor   See glPolygonOffset. Negative values give less
     *                          stitching, but too extreme can create other
     *                          rendering artifacts. Default = 0; means use
     *                          algorithm default.
     *
     * @param   offset_units    See glPolygonOffset. Negative values  give less
     *                          stitching, but too extreme can create other
     *                          rendering artifacts. Default = 0; means use
     *                          algorithm default.
     */
    Wire_occlude_renderer(const Generic_renderable* model, float offset_factor = 0, float offset_units = 0) :
        Generic_renderer(model),
        m_offset_factor(offset_factor),
        m_offset_units(offset_units)
    {}


    void set_factor(float f) { m_offset_factor = f; }
    void set_units(float u) { m_offset_units = u; }

    virtual void render(const Generic_renderable& model) const
    {
        model.wire_occlude_render(m_offset_factor, m_offset_units);
    }
protected:
    float m_offset_factor;
    float m_offset_units;
};

typedef Wire_occlude_renderer Silhouette_renderer;

/**
 * @class Solid_renderer
 *
 * Allows us to pass the solid wire rendering technique to a generic rendering function.
 *
 * @author Kyle Simek
 * */
class Solid_renderer : public Generic_renderer
{
public:
    /**
     * Note: caller is responsible for deleting model
     */
    Solid_renderer() : Generic_renderer() {}
    Solid_renderer(const Generic_renderable* model) :
        Generic_renderer(model) {}


    void render(const Generic_renderable& model) const
    {
        model.render();
    }
};

/**
 * Abstract class for objects that can be rendered by receiving a renderer object.
 *
 * This automatically provides all render methods of Generic_renderable.
 */
class Renderer_renderable : public Generic_renderable
{
public:
    virtual void render(const kjb::Generic_renderer& renderer) const = 0;

    void render() const
    {
        return render(kjb::Solid_renderer());
    }

    void solid_render() const
    {
        return render(kjb::Solid_renderer());
    }

    void wire_renderer() const
    {
        return render(kjb::Wire_renderer());
    }

    void wire_occlude_renderer() const
    {
        return render(kjb::Wire_occlude_renderer());
    }


};

// ///////////////////////////////////////////////////////
//    Multi-view renderables
//
//    These are mostly analagous to the Renderables above,
//    with the additional ability to represent multiple
//    views of a scene, and switch between the active view
//    to be rendered.
// ////////////////////////////////////////////////////////

/**
 * A renderable object that has multiple views.  Only one
 * view is active at any time, which is the view that
 * is rendered when render() is called.
 *
 * @brief Abstract class to render an object that has many possible views.
 */
class Mv_renderable : public Renderable
{
public:

    /** @brief Deletes this Renderable. */
    virtual ~Mv_renderable() { }

    /** @brief Returns the number of views for this object */
    virtual size_t num_views() const = 0;

    /** @brief Choose a view of this object to render.
     * Active model is not considered part of object state,
     * so this is const. */
    virtual void set_active_view(size_t i) const = 0;

    /** @brief Renders this object with GL. */
    virtual void render() const = 0;
};

/**
 * A multi-view renderable like Mv_renderable, but
 * adds solid, wire, and wire_occlude_render methods.
 */
class Mv_generic_renderable : public Mv_renderable, public Generic_renderable
{ };


/**
 * Wrap a multi-view renderable object so it's default
 * render() routine is wire_occlude_render().
 */
class Mv_wire_occlude_render_wrapper : public Mv_renderable
{
public:
    Mv_wire_occlude_render_wrapper(const Mv_generic_renderable* model,
            float offset_factor = 0, float offset_units = 0) :
        m_model(model),
        m_offset_factor(offset_factor),
        m_offset_units(offset_units)
    { }

    size_t num_views() const { return m_model->num_views(); }
    void set_active_view(size_t i) const { m_model->set_active_view(i); }

    void render() const
    {
        m_model->wire_occlude_render(m_offset_factor, m_offset_units);
    }
public:
    const Mv_generic_renderable* m_model;
    float m_offset_factor;
    float m_offset_units;
};

/**
 * Wrap a multi-view renderable object so it's default
 * render() routine is wire_render().
 */
class Mv_wire_render_wrapper : public Mv_renderable
{
public:
    Mv_wire_render_wrapper(const Mv_generic_renderable* model) :
        m_model(model)
    { }

    size_t num_views() const { return m_model->num_views(); }
    void set_active_view(size_t i) const { m_model->set_active_view(i); }

    void render() const
    {
        m_model->wire_render();
    }
public:
    const Mv_generic_renderable* m_model;
    float m_offset_factor;
    float m_offset_units;
};

/**
 * Wrap a multi-view renderable object so it's default
 * render() routine is solid_render().
 */
class Mv_solid_render_wrapper : public Mv_renderable
{
public:
    Mv_solid_render_wrapper(const Mv_generic_renderable* model) :
        m_model(model)
    { }

    size_t num_views() const { return m_model->num_views(); }
    void set_active_view(size_t i) const { m_model->set_active_view(i); }

    void render() const
    {
        m_model->solid_render();
    }
public:
    const Mv_generic_renderable* m_model;
    float m_offset_factor;
    float m_offset_units;
};

}


#endif
