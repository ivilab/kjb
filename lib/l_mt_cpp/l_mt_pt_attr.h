/**
 * @file
 * @author Andrew Predoehl
 * @brief def of RAII class to control a pthread attr object
 */
/*
 * $Id: l_mt_pt_attr.h 17433 2014-09-02 02:52:04Z predoehl $
 */

#ifndef PT_ATTR_H_INCLUDED_IVILAB
#define PT_ATTR_H_INCLUDED_IVILAB

#include <l_mt/l_mt_pthread.h>
#include <l_cpp/l_exception.h>

namespace kjb
{

/**
 * @addtogroup kjbThreads Thread support
 * @{
 */

/**
 * @brief RAII class to manage an object of type kjb_pthread_attr_t
 *
 * The kjb_pthread sublibrary, which wraps the Posix pthread library,
 * uses an object of type kjb_pthread_attr_t to describe the attributes
 * of a thread.  Thread attributes are its "detach state," its stack
 * size, and other things.  See pthread_attr_init(3).
 * A single attribute object may be used on multiple threads.
 *
 * The attribute object must be created and destroyed, thus this RAII
 * class exists to manage those tasks.
 * Objects of this class are not copyable or assignable.
 *
 * Currently this class does not support all thread attribute options,
 * but a developer is welcome to add new ones to the C library wrapper
 * (l_mt) and add WRAPPED calls through this class.  Please do not call
 * pthreads functions directly -- use the wrapped calls in kjb_c.
 *
 * Example.
 * @code
 * void* ha(void*) { for(int i=100; i--; ) puts("lol"); }
 * main() {
 *  kjb_c::kjb_pthread_t tid;
 *  kjb::Pthread_attr a;
 *  a.set_detached();
 *  kjb_c::kjb_pthread_create(*tid, a, ha, NULL);
 *  // no need to join
 *  return 0;
 * }
 * @endcode
 */
class Pthread_attr
{
    // private attribute object
    kjb_c::kjb_pthread_attr_t m_attr;

    Pthread_attr(const Pthread_attr&);  // teaser:  do not copy
    Pthread_attr& operator=(const Pthread_attr&);  // teaser:  do not assign

public:
    /// @brief initialize a generic attribute object.
    Pthread_attr()
    {
        ETX(kjb_c::kjb_pthread_attr_init(&m_attr));
    }

    /// @brief destroy an attribute object.
    ~Pthread_attr()
    {
        kjb_c::kjb_pthread_attr_destroy(&m_attr);
    }

    /**
     * @brief set the detach flag.
     * @param flag KJB_PTHREAD_CREATE_JOINABLE, KJB_PTHREAD_CREATE_DETACHED
     *             are the two valid values.
     * @return ERROR or NO_ERROR as appropriate
     * @see set_detached as a possibly more readable alternative
     */
    int setdetachstate(int state)
    {
        return kjb_c::kjb_pthread_attr_setdetachstate(&m_attr, state);
    }

    /// @brief convenience synonym setdetachstate(KJB_PTHREAD_CREATE_DETACHED)
    int set_detached()
    {
        return setdetachstate(KJB_PTHREAD_CREATE_DETACHED);
    }

    /**
     * @brief obtain the detach setting of this object.
     * @param[out] state  Pointer to int into which the state is written
     * @return ERROR or NO_ERROR
     * @post *state contains KJB_PTHREAD_CREATE_JOINABLE if the thread is
     *       joinable, or KJB_PTHREAD_CREATE_DETACHED if not.
     */
    int getdetachstate(int* state)
    {
        return kjb_c::kjb_pthread_attr_getdetachstate(&m_attr, state);
    }

    /// @brief access thread attribute as const pointer.
    operator const kjb_c::kjb_pthread_attr_t*()
    {
        return &m_attr;
    }
};

/// @}

} // end ns kjb

#endif /* PT_ATTR_H_INCLUDED_IVILAB */
