/**
 * @file
 * @brief XML processing prototypes
 * @author Scott Morris
 * @author Alan Morris
 *
 * Originally from TopoFusion.  This code enables GPX input and output.
 */
/*
 * $Id: xml.h 17606 2014-09-26 01:09:51Z predoehl $
 */

#ifndef XML_H_MORRISES_INCLUDED_UOFARIZONAVISION
#define XML_H_MORRISES_INCLUDED_UOFARIZONAVISION

#include <string>

#include <topo_cpp/layer.h>

namespace kjb
{
namespace TopoFusion
{

int readTrack_GPX( const std::string&, layer* );
int writeTrack_GPX( const std::string&, const layer& );

}
} // end namespace kjb

#endif
