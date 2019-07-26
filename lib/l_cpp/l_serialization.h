/* $Id: l_serialization.h 21596 2017-07-30 23:33:36Z kobus $ */
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

#ifndef KJB_L_SERIALIZATION_H
#define KJB_L_SERIALIZATION_H

#include "l/l_sys_def.h"
#include "l/l_sys_lib.h"
#include "l/l_sys_io.h"
#include "l_cpp/l_util.h"
#include "l_cpp/l_exception.h"
#include "l_cpp/l_index.h"

#include <string>
#include <iterator>
#include <fstream>

#include <boost/format.hpp>
#include <boost/utility/enable_if.hpp>
#include <boost/type_traits.hpp>


#ifdef KJB_HAVE_BST_SERIAL
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#endif

#include <boost/concept_check.hpp> 
#include <boost/concept_archetype.hpp>
#include <boost/static_assert.hpp>


// boost serialization of most 
namespace kjb {


/**
 * Concept check to ensure a type is serializable by boost.
 *
 * If your template function requires a serializable object, be sure to 
 * add a concept check using code like below.
 *
 * <code>
 * #include <boost/concept_check.hpp>
 * template <class T>
 * void my_func(T& my_object)
 * {
 * #ifdef KJB_HAVE_BST_SERIAL
 *      // check that T is serializable
 *      BOOST_CONCEPT_CHECK((kjb::SerializableConcept<T>));
 *
 *      ...
 * #else
 *      KJB_THROW_2(Missing_dependency, "boost::serialization");
 * #endif
 * }
 * </code>
 *
 * Note the double parentheses for BOOST_CONCEPT_CHECK, which are required,
 * as well as the guard statements for KJB_HAVE_BST_SERIAL.
 * Also don't forget the include statement for boost's concept check library.
 */
template <class X>
struct SerializableConcept
{
    BOOST_CONCEPT_USAGE(SerializableConcept)
    {
#ifdef KJB_HAVE_BST_SERIAL

        std::ofstream ofs("");
        std::ifstream ifs("");

        boost::archive::text_oarchive oa(ofs);
        boost::archive::text_iarchive ia(ifs);

        X* i = NULL;

        oa << *i;
        ia >> *i;
#else
        static const bool boost_serialization_not_available = true;
        BOOST_STATIC_ASSERT(boost_serialization_not_available == true);
#endif /* KJB_HAVE_BST_SERIAL */
    }
};

/** 
 * @brief Implements boost serialization for any object that implements the KjbReadableWritable concept.
 *
 * This function enables serialization for any Swappable class that can be constructed by receiving a filename, and implements a write(fname) method.  In general serialization should be implemented manually, but for kjb_c wrapped structures, manual wrapping is ill-advised for maintainability reasons.  This function makes those classes serializable.
 *
 * @warning This function is used by the boost library's serialization routines and should almost never be called directly.
 *
 * @note this function could have been called "serialize", which would allow boost to automatically serialize any classes modelling KjbReadableWritable.  However, since this function is rather ineffecient and not preferred if there are more sensible alternatives, we chose this function name so class authors are forced to explicitly call this function from the class's serialize() method.
 *
 * @see KjbReadableWritable_concept, Swappable_concept
 */
template <class Archive, class KJB_readable_writable>
void kjb_serialize_default(Archive & ar, KJB_readable_writable& obj, const unsigned int /* version */)
{
#ifdef KJB_HAVE_BST_SERIAL
    // This is pretty hacky.  We basically use the existing "write-to-file"/"read-from-file" semantics to convert the matrix to/from a string and then serialize the string representation.
    
    std::string obj_as_string;

    char tmp_fname[ MAXPATHLEN ];

    // create a legal and unique temporary file name
    KJB( ETX( BUFF_GET_TEMP_FILE_NAME( tmp_fname ) ) );
    // (this solution is not thread-safe
    // and could result in race conditions.  Evenutally, we
    // should write real serialization methods for vector, matrix, and
    // everything else that calls this template function. -- Kyle Apr 3, 2011

    if(Archive::is_saving::value == true)
    {
        obj.write(tmp_fname);

        std::ifstream ifs(tmp_fname);
  
        // read entire contents into text
        obj_as_string.assign(
                std::istreambuf_iterator<char>(ifs), 
                std::istreambuf_iterator<char>());

    }

    // do serialization of the string representation
    ar & obj_as_string;

    if(Archive::is_loading::value == true)
    {
        {
            //write entire file
            std::ofstream ofs(tmp_fname);
            ofs << obj_as_string;
        }

        // read file into m
        KJB_readable_writable tmp_matrix(tmp_fname);
        obj.swap(tmp_matrix);
    } 

    // delete tmp file
    ETX( kjb_c::kjb_unlink( tmp_fname ) );
#else /* KJB_HAVE_BST_SERIAL */
    KJB_THROW_2(Missing_dependency, "boost::serialization");
#endif /* KJB_HAVE_BST_SERIAL */
}

template <class Archive, class KJB_readable_writable>
void kjb_serialize(Archive & ar, KJB_readable_writable& obj, const unsigned int version)
{
    kjb_serialize_default(ar, obj, version);
}

/** 
 * Unserialize an object file.
 *
 * This assumes that:
 *  
 *  <li>The object was serialized by value
 *  <li>The object was saved to an ascii format using text_oarchive, 
 *  <li>The file contains only this object
 *
 *  All three of these criteria are satisfied by the kjb::save() function.
 *  Assuming the serialization routine and equality operator are implemented
 *  correctly, the test below should always pass:
 *
 *  <code>
 *  My_obj o;
 *  My_obj o2 = o;
 *
 *  save(o, "filename");
 *  load(o2, "filename");
 *  assert(o == o2);
 *  </code>
 *
 *  @author Kyle Simek
 */
template <class Serializable>
#ifdef KJB_HAVE_BST_SERIAL
void load(Serializable& obj, const std::string& fname)
#else
void load(Serializable&, const std::string&)
#endif
{
#ifdef KJB_HAVE_BST_SERIAL
    // consider adding load(stream, object), also

    BOOST_CONCEPT_ASSERT((SerializableConcept<Serializable>));
   
    std::ifstream ifs(fname.c_str());
    if(ifs.fail())
    {
        KJB_THROW_3(IO_error, "Error opening %s.", (fname.c_str()));
    }

    try
    {
        boost::archive::text_iarchive ia(ifs);
        ia >> obj;
    }
    catch(boost::archive::archive_exception& ex)
    {
        KJB_THROW_3(IO_error, "Error reading %s. Format may be invalid.  Boost error was: %s", (fname.c_str())(ex.what()));
    }

#else
        KJB_THROW_2(Missing_dependency, "boost::serialization");
#endif
}

/**
 * Load a sequence of files into a collection.
 *
 * @tparam SerializableOutputIterator Iterator whose value_type is Serializable
 *
 * @param iterator to first element of collection that will receive the loaded files
 * @param fmt_str Printf-formatted string representing the file sequence.  Must include some form of "%d" string.
 * @param indices An Index_range describing which files in the sequence to load.  As an alternative to passing an Index_range explicity, you may pass any of the common representations listed below.
 *
 * @note indices may be passed any of the following:
 *      <li> std::vector of type convertible to size_t
 *      <li> std::list of type convertible to size_t
 *      <li> kjb::Int_vector
 *      <li> Matlab-formatted string (i.e. "1:2:100")
 *      <li> int or size_t
 */
template <class SerializableOutputIterator>
void load_many(SerializableOutputIterator it, const std::string& fmt_str, const Index_range& indices)
{
    BOOST_CONCEPT_ASSERT((SerializableConcept<
                typename std::iterator_traits<SerializableOutputIterator>::value_type
    >));

    const size_t size = indices.size();
    boost::format fmt(fmt_str);
    for(size_t ii = 0; ii < size; ii++)
    {
        size_t i = indices[ii];

        std::string fname = (fmt % i).str();
        load(*it++, fname);
    }
}

/**
 * Load a sequence of files into a collection.
 *
 * @tparam SerializableOutputIterator Iterator whose value_type is Serializable
 *
 * @param iterator to first element of collection that will receive the loaded files
 * @param fmt_str Printf-formatted string representing the file sequence.  Must include some form of "%d" string.
 * @param num_files Number of files to read.
 * @param first_index First index of file to read (usually 0 or 1).
 */
template <class SerializableOutputIterator>
void load_many(SerializableOutputIterator it, const std::string& fmt_str, size_t num_files, size_t first_index = 0, size_t modulo = 0)
{
    BOOST_CONCEPT_ASSERT((SerializableConcept<
                typename std::iterator_traits<SerializableOutputIterator>::value_type
    >));

    Index_range indices(
            first_index,
            first_index + num_files - 1,
            modulo);

    load_many(it, fmt_str, indices);
}

template <class Value_type, class SerializablePtrOutputIterator>
void load_many_dynamic_dispatch(Value_type* /* trash */, SerializablePtrOutputIterator it, const std::string& fmt_str, const Index_range& indices )
{
    BOOST_CONCEPT_ASSERT((SerializableConcept<Value_type>));

    boost::format fmt(fmt_str);
    const size_t size = indices.size();
    for(size_t ii = 0; ii < size; ii++)
    {
        size_t i = indices[ii];

        Value_type* ptr = new Value_type();

        std::string fname = (fmt % i).str();
        load(*ptr, fname);

        *it++ = ptr;
    }
}

template <class Value_type, class SerializablePtrOutputIterator>
void load_many_dynamic_dispatch(typename boost::shared_ptr<Value_type> /* dummy */, SerializablePtrOutputIterator it, const std::string& fmt_str, const Index_range& indices )
{
    BOOST_CONCEPT_ASSERT((SerializableConcept<Value_type>));

    boost::format fmt(fmt_str);
    const size_t size = indices.size();
    for(size_t ii = 0; ii < size; ii++)
    {
        size_t i = indices[ii];
        boost::shared_ptr<Value_type> ptr(new Value_type());

        std::string fname = (fmt % i).str();
        load(*ptr.get(), fname);

        *it++ = ptr;
    }
}


template <class SerializablePtrOutputIterator>
void load_many_to_ptr(SerializablePtrOutputIterator it, const std::string& fmt_str, size_t num_files, size_t first_index = 0, size_t modulo = 1)
{
    typedef typename std::iterator_traits<SerializablePtrOutputIterator>::value_type Ptr_type;

    Index_range indices(first_index,
            first_index + num_files - 1,
            modulo);

    load_many_dynamic_dispatch(*it, it, fmt_str, indices);
}

/**
  @note indices may be any of the following:
       <li> std::vector of type convertible to size_t
       <li> std::list of type convertible to size_t
       <li> kjb::Int_vector
       <li> Matlab-formatted string (i.e. "1:2:100")
       <li> int or size_t
 */
template <class SerializablePtrOutputIterator, class Index_range_type>
typename boost::enable_if<boost::is_convertible<Index_range_type, Index_range>,void>::type
load_many_to_ptr(SerializablePtrOutputIterator it, const std::string& fmt_str, const Index_range_type& indices)
{
    typedef typename std::iterator_traits<SerializablePtrOutputIterator>::value_type Ptr_type;

    load_many_dynamic_dispatch(*it, it, fmt_str, indices);
}

//
//template <class Value_type, class SerializablePtrOutputIterator>
//void load_many_dynamic_dispatch(Value_type asdf, SerializablePtrOutputIterator it, const std::string& fmt_str, size_t num_files, size_t first_index)
//{
//    // error
//}


/** 
 * Write a serializable object to a file.
 *
 * This function will serialize the object by value
 * to an ascii format using text_oarchive.  It can be
 * loaded using the kjb::load() method, or by manually
 * using a boost::serialization text_iarchive object.
 *
 *  @author Kyle Simek
 */
template <class Serializable>
#ifdef KJB_HAVE_BST_SERIAL
void save(const Serializable& obj, const std::string& fname)
#else
void save(const Serializable&, const std::string&)
#endif
{
#ifdef KJB_HAVE_BST_SERIAL
    // consider adding save(object, stream), also
    BOOST_CONCEPT_ASSERT((SerializableConcept<Serializable>));
   
    std::ofstream ofs(fname.c_str());
    if(ofs.fail())
    {
        KJB_THROW_3(IO_error, "Error opening %s.", (fname.c_str()));
    }

    try
    {
        boost::archive::text_oarchive oa(ofs);
        oa << obj;
    }
    catch(...)
    {
        KJB_THROW_3(IO_error, "Error serializing object to file %s. Probably a bug in the serialization routine.", (fname.c_str()));
    }
#else
        KJB_THROW_2(Missing_dependency, "boost::serialization");
#endif
}



}

#endif
