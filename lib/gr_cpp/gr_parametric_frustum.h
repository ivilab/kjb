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
|     Joseph Schlecht, Luca Del Pero
|
* =========================================================================== */

/**
 * @file
 *
 * @author Joseph Schlecht, Luca Del Pero
 *
 * @brief frustum: a hexahedron of which each face is a parallelegram.
 */


#ifndef KJB_Frustum_H
#define KJB_Frustum_H

#include <vector>
#include <iosfwd>

#include <inttypes.h>

#include <gr_cpp/gr_polymesh.h>
#include <gr_cpp/gr_polymesh_renderer.h>

namespace kjb {


/** 
 * @class Frustum
 *
 * @brief frustum: a polyhedron of which each torso face is a trapezoid and the top and bottom surfaces are polygons.
 */
class Frustum : public kjb::Polymesh
{
    public:

        /** @brief Constructs a frustum.
         *
         *
         */
        Frustum
        (
            unsigned int inv,
            double ix, double iy, double iz,
            double iw, double il, double iratio_top_bottom, double ih,
            double ipitch, double iyaw, double iroll
        );


        /** @brief Constructs a */
        Frustum
        (
            const std::vector<kjb::Vector> & p
        )
        throw (kjb::Illegal_argument);


        /** @brief Constructs a frustum by copying another. */
        Frustum(const Frustum& p);


        /** @brief Reads a frustum from an input file. */
        Frustum(const char* fname, unsigned int inv) throw (kjb::Illegal_argument,
                kjb::IO_error);


        /** @brief Reads a frustum from an input stream. */
        Frustum(std::istream& in, unsigned int inv) throw (kjb::Illegal_argument,
                kjb::IO_error);


        /** @brief Deletes this frustum. */
        virtual ~Frustum() { };


        /** @brief Copies a frustum into this one. */
        virtual Frustum& operator= (const Frustum& p);


        /** @brief Clones this frustum. */
        virtual Frustum* clone() const;

        /** @brief Transforms this frustum */
        virtual void transform(const kjb::Matrix & M)
            throw (kjb::Illegal_argument);


        /** @brief Returns an indexed point defining this Frustum. */
        const kjb::Vector & get_point(size_t i) const
            throw (kjb::Illegal_argument);


        /** @brief Returns the center vector for this frustum. */
        const kjb::Vector & get_center() const;

        /** @brief Adds a face to this Frustum -> Not implemented here,
         * it will throw an exception, because the use of this method
         * will violate the constraints defining this Frustum
         */
        virtual void add_face(const Polygon & face) throw (kjb::Illegal_argument);


        /** @brief Reads this frustum from an input stream. */
        virtual void read(std::istream& in) throw (kjb::IO_error,
                kjb::Illegal_argument);

        /** @brief Writes this frustum to an output stream. */
        virtual void write(std::ostream& out) const
            throw (kjb::IO_error);

        /** @brief reset the points of this Frustum */
        void set_points
        (
            unsigned int inv,
            double ix, double iy, double iz,
            double iw, double il, double iratio_top_bottom, double ih
        );

        /** @brief returns the index of the face adjacent to face f along edge e
         *  This is an efficient implementation to be used only in the context
         *  of the frustum
         */
        virtual unsigned int adjacent_face(unsigned int f, unsigned int e) const
                       throw (Index_out_of_bounds,KJB_error);

        void draw_orientation_map() const;

/*        void draw_left_right_orientation_map() const;

        void draw_geometric_context_map() const;*/

        static int get_num_edges(int num_facets)
        {
            //KJB_THROW(kjb::Not_implemented);
            return 0; //TODO implement
        }

        static int get_edge_indexes
        (
            std::vector<int> & base_edge_indexes,
            std::vector<int> & vertical_edge_indexes,
            std::vector<int> & top_edge_indexes,
            int num_facets
        )
        {
            //TODO implement
            //KJB_THROW(kjb::Not_implemented);
            return 0;
        }

    protected:

        /** @brief number of vertices for the top(bottom) surface. */
        unsigned int nv;

        /** @brief Points defining this Frustum. */
        std::vector<kjb::Vector > points;

        /** 
         * @brief Center of this frustum.
         * 
         * Calculated as the average of the face centroids.
         */
        kjb::Vector center;

    private:

        /**
        * @brief it stores information on face adjacency
        * for efficiency reasons
        */
       Int_matrix _adjacency;

       /**
        * It fills the adjacency matrix for this Frustum
        */
       void create_adjacency_matrix();

};


}


#endif
