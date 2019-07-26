/* $Id: l_cpp_bits.h 21596 2017-07-30 23:33:36Z kobus $ */
/* {{{=========================================================================== *
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
 * =========================================================================== }}}*/

// vim: tabstop=4 shiftwidth=4 foldmethod=marker

#ifndef KJB_CPP_L_BITS
#define KJB_CPP_L_BITS

#include "l/l_bits.h"
#include "l_cpp/l_exception.h"

namespace kjb
{

/**
 * @brief Swap the byte-order of a value.
 *
 * @note You shouldn't call this directly, you should call swap_bytes.
 * @tparam NUM_BYTES number of bytes in the value.
 *
 */
template <int NUM_BYTES>
inline void swap_bytes_dispatch(void* /* value */)
{
    KJB_THROW(Not_implemented);
}

/// @brief swap the byte order of a 16-bit value (do not call directly).
template<>
inline void swap_bytes_dispatch<2>(void* value)
{
    kjb_c::bswap_u16(static_cast<uint16_t*>(value));
}

/// @brief swap the byte order of a 32-bit value (do not call directly).
template<>
inline void swap_bytes_dispatch<4>(void* value)
{
    kjb_c::bswap_u32(static_cast<uint32_t*>(value));
}

/// @brief swap the byte order of a 64-bit value (do not call directly).
template<>
inline void swap_bytes_dispatch<8>(void* value)
{
    kjb_c::bswap_u64(static_cast<uint64_t*>(value));
}

/**
 * Reverse a value's byte-order.  Useful when swapping the endian-ness
 * of a value.  
 *
 * This is written so that the compiler should convert this into a direct call 
 * to the high-performance C-version, bswap_u*().
 *
 * This template version makes the byte swapping functionality usable in generic
 * algorithms, as well as unifying the interface somewhat.
 *
 * @param value The value whose bytes to swap
 *
 * @note To be used only with primitive types of 2, 4, or 8 bytes size.
 * @warning Not to be used with bool.
 *
 * @author Kyle Simek
 */
template <class T>
inline void swap_bytes(T* value)
{
    swap_bytes_dispatch<sizeof(T)>((void*) value);
}

template <class T>
inline void swap_bytes(T& value)
{
    swap_bytes_dispatch<sizeof(T)>((void*) &value);
}


/**
 * Reverse the byte-order of an array of values.  Useful when swapping endian-ness
 * of an array of values. This should be used only with arrays of primitive types 
 * of 1 word or less.
 *
 * @autor Kyle Simek
 */
template <class T>
void swap_array_bytes(T* array, size_t N)
{
    for(size_t i = 0; i < N; ++i)
    {
        swap_bytes(array++);
    }
}

} // namespace kjb

#endif
