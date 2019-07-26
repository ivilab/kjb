/* $Id: gr_sprite.h 18278 2014-11-25 01:42:10Z ksimek $ */
/* =========================================================================== *
   |
   |  Copyright (c) 1994-2010 by Kobus Barnard (author)
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
   |  Author:  Kyle Simek
 * =========================================================================== */

#ifndef KJB_sprite_H
#define KJB_sprite_H


#include <gr_cpp/gr_opengl_texture.h> /* comes before other opengl includes */
#include <i_cpp/i_image.h>
#include <gr_cpp/gr_renderable.h>
#include <gr_cpp/gr_opengl_headers.h>
#include <map>

namespace kjb
{

namespace opengl
{

/**
 *  A simple class for displaying bitmap images using opengl
 *
 *  All dimensions are expressed in screen coordinates.  By default, the image 
 *  is displayed at its native resolution (one image pixel equals one screen 
 *  pixel), but it can be stretched by setting a percentage scale or an absolute 
 *  width and height.  By default, sprite rendering ignores the z-buffer, but 
 *  this behavior can be changed using enable_depth_test.
 *
 *  @author Kyle Simek
 */
#ifdef  KJB_HAVE_OPENGL
    class Sprite : public Renderable
    {
        enum Origin_type {TOP_LEFT, CENTER};
        public:
            Sprite (float z_pos = 1);
            Sprite (const Image& img, float z_pos = 1);
            Sprite (const Matrix& gray_img, float z_pos = 1);
            Sprite (const Texture& img, float z_pos = 1);
//            Sprite (const char* fname, float z_pos = 1);
            Sprite ( const Sprite& other );
            ~Sprite ();

            void swap(Sprite& src)
            {
                std::swap(m_texture, src.m_texture);
                std::swap(m_u, src.m_u);
                std::swap(m_v, src.m_v);
                std::swap(m_base_width, src.m_base_width);
                std::swap(m_base_height, src.m_base_height);
                std::swap(m_width, src.m_width);
                std::swap(m_height, src.m_height);
                std::swap(m_depth_test, src.m_depth_test);
                std::swap(m_min_filter, src.m_min_filter);
                std::swap(m_mag_filter, src.m_mag_filter);
                std::swap(m_z_pos, src.m_z_pos);
            }

            void enable_depth_test(bool enable);
            void set_filters(GLenum min_filter, GLenum mag_filter);

            void set_position(int u, int v) {
                set_u(u);
                set_v(v);
            }

            void set_u(int u) { m_u = u; }
            void set_v(int v) { m_v = v; }

            /**
             * Set crop boundaries in relative coordinates, scaled between [0, 1].
             */
            void set_crop_relative(float left, float right, float bottom, float top)
            {
                m_crop_left = left;
                m_crop_right = right;
                m_crop_top = top;
                m_crop_bottom = bottom;
            }

            /**
             * Set crop boundaries in pixel units, relative to the UNSCALED dimensions of the image
             */
            void set_crop_absolute(float left, float right, float bottom, float top)
            {
                m_crop_left   = left   / m_base_width;
                m_crop_right  = right  / m_base_width;
                m_crop_bottom = bottom / m_base_height;
                m_crop_top    = top    / m_base_height;
            }

            /** @brief Set rectangle size using size relative to the underlying image resolution */
            void set_scale(float u_scale, float v_scale)
            {
                m_width = (int) (m_base_width * u_scale);
                m_height = (int)(m_base_height * v_scale);
            }

            /** @brief Set absolte rectangle size in screen pixels */
            void set_size(int width, int height)
            {
                m_width = width;
                m_height = height;
            }

            /** @brief Set absolte rectangle size and position in screen pixels */
            void set_rect(int u, int v, int width, int height) {
                set_position(u,v);
                set_size(width, height);
            }

            /** @brief Get resolution of the underlying image */
            int get_base_width() const { return m_base_width; }
            int get_base_height() const {return m_base_height; }

            /** @brief Get current size of rectangle */
            int get_width() const { return m_width; }
            int get_height() const { return m_height; }

            /** @brief Get scaling factor of the current rectangle */
            int get_u_scale() const { return m_width / (float) m_base_width; }
            int get_v_scale() const { return m_height / (float) m_base_height; }

            int get_u() const { return m_u;}
            int get_v() const { return m_v;}
            Vector get_position() const { 
                Vector pos(3); 
                pos[0] = m_u; pos[1] = m_v; pos[2] = 0;
                return pos;
            }

            /**
             * set image origin to be in the center of the image instead
             * of the default top-left corner
             */
            void center_origin()
            {
                m_origin = CENTER;
            }

            /** 
             * treat sprite like a polygon in world space.
             * returns vertex position in world coordinates.
             **/
/*             virtual const Vector& get_world_vertex(int i) const;
 */

            virtual void render() const;
            virtual void render_quad() const;

            Sprite& operator=(const Sprite &src);

            void set(const Image& img);
            void set(const Texture& tex);
            void set(const Matrix& img);

            const Texture& get_texture() const
            {
                return m_texture;
            }

        protected:
            void m_init_texture();
        protected:
            Texture m_texture;
            static std::map<int, int> m_ref_count;

            int m_u, m_v;

            int m_base_width;
            int m_base_height;

            int m_width, m_height;

            bool m_depth_test;
            GLenum m_min_filter;
            GLenum m_mag_filter;

            float m_z_pos;

            Origin_type m_origin;

            float m_crop_left;
            float m_crop_right;
            float m_crop_bottom;
            float m_crop_top;

    }; // class Sprite

    typedef boost::shared_ptr<Sprite> Sprite_ptr;

//    class Packed_sprite : public Sprite
//    {
//        typedef Sprite Base;
//        typedef Packed_sprite Self;
//        public:
//            Packed_sprite (float z_pos = 1) : 
//                Sprite(z_pos)
//            {}
//
//            Packed_sprite (const Matrix& mat, float z_pos = 1) :
//                Sprite(z_pos)
//            {
//                m_set(mat);
//            }
//
//            Packed_sprite ( const Packed_sprite& other ) :
//                Sprite(other)
//            {}
//
//            ~Packed_sprite()
//            {}
//
//            Packed_sprite& operator=(const Sprite &src)
//            {
//                Base::operator=(src);
//                return *this;
//            }
//
//
//        private:
//            void m_set(const Matrix& mat);
//    }; // class Sprite

#endif     /* KJB_HAVE_OPENGL */

} // namespace opengl


#ifdef  KJB_HAVE_OPENGL
/** 
 * Deprecated.  Sprite is now under the opengl namespace, making the 'Gl_' prefix superfluous.  Use kjb::opengl::Sprite.
 */
typedef opengl::Sprite Gl_sprite;
#endif

} // namespace kjb
#endif /* ----- #ifndef KJB_sprite_H  ----- */
