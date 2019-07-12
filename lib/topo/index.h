/**
 * @file
 * @brief Functions and structures specific to file and memory pool indexing.
 * @author Scott Morris
 * @author Alan Morris
 *
 * Originally from TopoFusion.
 */
/*
 * $Id: index.h 15651 2013-10-11 00:38:52Z kobus $
 *
 * Recommended tab width:  4
 */

#ifndef INDEX_H_INCLUDED_UOFARIZONAVISION
#define INDEX_H_INCLUDED_UOFARIZONAVISION

#include "l/l_sys_def.h"

#ifdef __cplusplus
extern "C" {
#ifdef COMPILING_CPLUSPLUS_SOURCE
namespace kjb_c {
namespace TopoFusion {
#endif
#endif

int initIndexFile( void );

void closeIndexFile( void );

int addIndexEntry( int, int, char, char, const char*, int );

int findTileOnDisk( int x, int y, int typ, char zone );

int getTileFromDisk( int x, int y, int typ, char zone, char *buf, size_t );

int invalidateEntry( int x, int y, int typ, char zone );

#ifdef __cplusplus
#ifdef COMPILING_CPLUSPLUS_SOURCE
} /* namespace TopoFusion */
} /* namespace kjb_c */
#endif
} /* extern "C" */
#endif


#endif  /* INDEX_H_INCLUDED_UOFARIZONAVISION */

