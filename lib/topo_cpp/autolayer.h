/**
 * @file
 * @brief Wrapper for Scott Morris's layer structure
 * @author Andrew Predoehl
 */
/*
 * $Id: autolayer.h 17606 2014-09-26 01:09:51Z predoehl $
 */

#ifndef AUTOLAYER_H_UOFARIZONAVISION
#define AUTOLAYER_H_UOFARIZONAVISION

#include <l_cpp/l_util.h>
#include <l_cpp/l_exception.h>
#include <topo_cpp/layer.h>
#include <topo_cpp/xml.h>

#include <string>

namespace kjb
{
namespace TopoFusion
{

/// @brief RAII approach to the old TopoFusion Layer data structure
class AutoLayer
{
    layer my_layer; ///< this is what the wrapper is encapsulating

    /// @brief reset the name (of file, usually) associated with a layer
    int set_name( const std::string& name )
    {
        if ( name.size() >= LAYER_FN_SIZE )
        {
            return kjb_c::ERROR;
        }
        strcpy( my_layer.filename, name.c_str() );
        return kjb_c::NO_ERROR;
    }

    void set_numTracks( int );

    AutoLayer( const AutoLayer& );  ///< teaser: there is no copy ctor

public:
    /// @brief default ctor makes a Layer ready for writing
    AutoLayer()
    {
        initLayer( &my_layer );
    }

    /// @brief ctor loads a layer from a named file
    AutoLayer( const std::string& filename )
    {
        initLayer( &my_layer );
        KJB( ETX( readTrack_GPX( filename, &my_layer ) ) );
    }


    pt* set_track0points( unsigned, const pt* src=00 );


    void add_waypoint( const pt&, const char*, int );


    /// @brief this cheat lets us use an AutoLayer like a const Layer object
    operator const layer& () const
    {
        return my_layer;
    }

    /// @brief This is a "dangerous" method that lets you manipulate the Layer
    layer* get_writable_ptr()
    {
        return &my_layer;
    }

    /// @brief Write a layer into a file
    int write( const std::string& filename )
    {
        KJB( ETX( set_name( filename ) ) );
        KJB( ETX( writeTrack_GPX( filename, my_layer ) ) );
        return kjb_c::NO_ERROR;
    }

    /// @brief Dtor makes sure the layer resources are released
    ~AutoLayer()
    {
        destroyLayer( &my_layer );
    }
};

}
} // end namespace kjb


#endif /* autolayer.h */
