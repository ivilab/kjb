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
 * @brief Parallelepiped: a hexahedron of which each face is a parallelegram.
 */


#ifndef KJB_PARAPIPED_H
#define KJB_PARAPIPED_H

#include <iosfwd>
#include <vector>

#include <inttypes.h>

#include <gr_cpp/gr_polymesh.h>
#include <gr_cpp/gr_polymesh_renderer.h>
#include <l_cpp/l_int_matrix.h>

namespace kjb {


/** 
 * @class Parapiped
 *
 * @brief Parallelepiped: a hexahedron of which each face is a parallelegram.
 */
class Parapiped : public kjb::Polymesh
{
    public:

        /** @brief Constructs a parallelepiped. */
        Parapiped
        (
            double x1, double y1, double z1,
            double x2, double y2, double z2,
            double x3, double y3, double z3,
            double x4, double y4, double z4
        );


        /** @brief Constructs a parallelepiped. */
        Parapiped
        (
            const kjb::Vector & p1,
            const kjb::Vector & p2,
            const kjb::Vector & p3,
            const kjb::Vector & p4
        )
        throw (kjb::Illegal_argument);


        /** @brief Constructs a parallelepiped by copying another. */
        Parapiped(const Parapiped& p);


        /** @brief Reads a parallelepiped from an input file. */
        Parapiped(const char* fname) throw (kjb::Illegal_argument,
                kjb::IO_error);


        /** @brief Reads a parallelepiped from an input stream. */
        Parapiped(std::istream& in) throw (kjb::Illegal_argument,
                kjb::IO_error);


        /** @brief Deletes this parallelepiped. */
        virtual ~Parapiped() { };


        /** @brief Copies a parallelepiped into this one. */
        virtual Parapiped& operator= (const Parapiped& p);


        /** @brief Clones this parallelepiped. */
        virtual Parapiped* clone() const;

        /** @brief Transforms this parallelepiped */
        virtual void transform(const kjb::Matrix & M)
            throw (kjb::Illegal_argument);


        /** @brief Returns an indexed point defining this parapiped. */
        const kjb::Vector & get_point(size_t i) const
            throw (kjb::Illegal_argument);


        /** @brief Returns the center vector for this parallelepiped. */
        const kjb::Vector & get_center() const;

        /** @brief Adds a face to this parapiped -> Not implemented here,
         * it will throw an exception, because the use of this method
         * will violate the constraints defining this parapiped
         */
        virtual void add_face(const Polygon & face) throw (kjb::Illegal_argument);


        /** @brief Reads this parallelepiped from an input stream. */
        virtual void read(std::istream& in) throw (kjb::IO_error,
                kjb::Illegal_argument);


        /** @brief Writes this parallelepiped to an output stream. */
        virtual void write(std::ostream& out) const
            throw (kjb::IO_error);

        /** @brief reset the points of this parapiped */
        void set_points
        (
            double x1, double y1, double z1,
            double x2, double y2, double z2,
            double x3, double y3, double z3,
            double x4, double y4, double z4
        );

        /** @brief returns the index of the face adjacent to face f along edge e
         *  This is an efficient implementation to be used only in the context
         *  of the parallelepiped
         */
        virtual unsigned int adjacent_face(unsigned int f, unsigned int e) const
                       throw (Index_out_of_bounds,KJB_error);

        void draw_orientation_map() const;

        void draw_left_right_orientation_map() const;

        void draw_CMU_orientation_map() const;

        void draw_geometric_context_map() const;

        static int get_num_edges()
        {
            return 24;
        }

        static void get_edge_indexes
        (
            std::vector<int> & base_edge_indexes,
            std::vector<int> & vertical_edge_indexes,
            std::vector<int> & top_edge_indexes
        )
        {
            base_edge_indexes.push_back(2);
            base_edge_indexes.push_back(4);
            base_edge_indexes.push_back(12);
            base_edge_indexes.push_back(13);
            base_edge_indexes.push_back(14);
            base_edge_indexes.push_back(15);
            base_edge_indexes.push_back(18);
            base_edge_indexes.push_back(23);

            //TODO Double check this
            vertical_edge_indexes.push_back(1);
            vertical_edge_indexes.push_back(3);
            vertical_edge_indexes.push_back(5);
            vertical_edge_indexes.push_back(7);
            vertical_edge_indexes.push_back(17);
            vertical_edge_indexes.push_back(19);
            vertical_edge_indexes.push_back(20);
            vertical_edge_indexes.push_back(22);

            top_edge_indexes.push_back(0);
            top_edge_indexes.push_back(6);
            top_edge_indexes.push_back(8);
            top_edge_indexes.push_back(9);
            top_edge_indexes.push_back(10);
            top_edge_indexes.push_back(11);
            top_edge_indexes.push_back(16);
            top_edge_indexes.push_back(21);
        }

    protected:

        /** @brief Points defining this parapiped. */
        std::vector<kjb::Vector > points;

        /** 
         * @brief Center of this parallelepiped.
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
        * It fills the adjacency matrix for this parapiped
        */
       void create_adjacency_matrix();

};


}


#endif
