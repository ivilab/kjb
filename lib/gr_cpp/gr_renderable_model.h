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
|     Luca Del Pero
|
* =========================================================================== */


/**
 * @file
 *
 * @author Luca Del Pero
 *
 * @brief St_renderable_model Base class for a structure model with a renderable
 *        representation
 *
 */


#ifndef GR_RENDERABLE_MODEL_H_
#define GR_RENDERABLE_MODEL_H_

#include <gr_cpp/gr_renderable.h>
#include <l_cpp/l_cloneable.h>

namespace kjb
{
    class Base_gl_interface;

    /*
     * @class Renderable_model This class implements basic functionalities for a parametric model
     * with the capability of being renderable. This assumes that a model has some parameters and
     * a representation used for rendering that need to be consistent with the value of
     * the model parameters. Since updating the rendering interface to match the model
     * parameters is usually expensive, we do it only when it is strictly necessary, that is
     * to say before calling any rendering method. This class provides basic rendering
     * methods (wireframe, solid, wireframe with occluded edges removed) that check whether
     * the rendering interface is updated before calling the appropriate rendering method,
     * and update it if it is the case
     */
    class Renderable_model: public Cloneable
    {
    public:

        /** @brief Constructor
         *
         * @istatus specifies whether the rendering interface matches the model parameters
         *          or needs to be updated
         */
        Renderable_model(bool istatus = false) : Cloneable()
        {
            _rendering_representation_updated = istatus;
        }

        /** @brief Copy constructor */
        Renderable_model(const Renderable_model & src) : Cloneable()
        {
            (*this) = src;
        }

        /** @brief Assignment operator */
        inline Renderable_model & operator=(const Renderable_model & /* src */)
        {
            _rendering_representation_updated = false;
            return (*this);
        }

        virtual ~Renderable_model() { }

        /** @brief Renders this model as a wireframe */
        virtual void wire_render() const throw(kjb::KJB_error);
        /** @brief Renders this model into the depth buffer */
        virtual void wire_occlude_render() const throw(kjb::KJB_error);
        /** @brief Renders this model as a solid */
        virtual void solid_render() const throw(kjb::KJB_error);
        /** @brief Renders this model as a wireframe by removing occluded edges */
        virtual void render_occluded_wireframe() const throw(kjb::KJB_error);

        /** @brief renders the silhouette of this object */
        virtual void silhouette_render(const kjb::Base_gl_interface &, double width = 1.0) const;

        /** Updates the graphical representation if needed */
        inline void update_if_needed() const
        {
            if(!_rendering_representation_updated)
            {
                update_rendering_representation();
                _rendering_representation_updated = true;
            }
        }

        inline void force_update() const
        {
            update_rendering_representation();
            _rendering_representation_updated = true;
        }

    protected:
        /** @brief Returns a reference to the rendering interface used to render this model */
        virtual Abstract_renderable & get_rendering_interface() const = 0;

         /** @brief Updates the rendering interface so that it reflects the paremeter
          * values of this model to the rendering interface used to render this model */
        virtual void update_rendering_representation() const throw(kjb::KJB_error) = 0;

        /** @brief This method is called to whenever the model parameters where changed
         * without updating the rendering interface accordingly. This means that the
         * rendering interface has to be updated before calling any rendering method,
         * otherwise the rendering will not match the parameter values of the model
         */
        inline void set_rendering_representation_dirty() const
        {
            _rendering_representation_updated = false;
        }

    private:
        /** Set to true when the rendering interface matches the values of the model paremetersn
         * to false otherwise
         */
        mutable bool _rendering_representation_updated;
    };
}

#endif
