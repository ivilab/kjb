/* $Id $ */

/**
 * @file
 *
 * This header exists for backward compatibility.  It simply includes:
 *
 * * gr_cpp/gr_line_segment.h
 * * edge_cpp/edge_segment.h
 *
 * Copyright (c) 2003-2008 by members of University of Arizona Computer Vision
 * group. For use outside the University of Arizona Computer Vision group
 * please contact Kobus Barnard.
 */



#ifndef EDGE_LINE_SEGMENT_H
#define EDGE_LINE_SEGMENT_H

// refactored to break the dependency between gr_cpp and edge_cpp.
// Moved the line_segment class into gr_cpp, which allowed it to be used in that
// sublibrary without introducing a dpeendency on the edge_cpp sublibrary.
//
// This shouldn't affect existing code that includes this header.
// -- Kyle  November 23 2014 

namespace kjb
{
    class Segment_pair;
}
#include <gr_cpp/gr_line_segment.h>
#include <edge_cpp/edge_segment.h>

#endif
