/**
 * @file
 * @author Andrew Predoehl
 * @brief Wrapper for message digest (hash) interface provided by OpenSSL
 *
 * Please see class kjb::OpenSSL_EVP and class kjb::MD5.
 */
/*
 * $Id: wrap_openssl_evp.h 17473 2014-09-08 01:22:34Z predoehl $
 */

#ifndef OPENSSL_CPP_EVP_H_WRAP_PREDOEHL_UOFARIZONA_VISION
#define OPENSSL_CPP_EVP_H_WRAP_PREDOEHL_UOFARIZONA_VISION

#include <l_cpp/l_exception.h>

#ifdef KJB_HAVE_OPENSSL
#include <openssl/evp.h>
#else
#warning "OpenSSL is required for class OpenSSL_EVP to function properly"
#endif

#include <vector>


namespace kjb
{

/**
 * @brief Generic OpenSSL hasher class, the base class for specific derivations
 *
 * @throws kjb::Missing_dependency if OpenSSL library was missing when compiled
 */
class OpenSSL_EVP
{

public:
    /// @brief The basic unit of both input and output for the hasher
    typedef unsigned char Unit;

    /// @brief Type of output of the hasher
    typedef std::vector< Unit > Digest;

private:

#ifdef KJB_HAVE_OPENSSL
    EVP_MD_CTX m_context;
#endif

    Digest m_digest;
    bool m_finalized;

    void finalize(); // not very long, but too long (I deem) to inline a lot

    void safe_finalize()
    {
        /*
         * I'd like to write this like so:      m_finalized || finalize();
         * but that would be poor sport.
         */
        if ( ! m_finalized )
        {
            finalize();
        }
    }

#ifdef KJB_HAVE_OPENSSL
    void update( const void* data, unsigned long len )
    {
        ETX_2( m_finalized, "Cannot write to an already-finalized digest");
        ETX( 0 == EVP_DigestUpdate( &m_context, data, len ) );
    }
#else
    /*
     * Since the class cannot be instantiated, you never get to here.
     * Unless something is horribly wrong, in which case, say that.
     *
     * If we had parameter names in the param list, it would cause warnings.
     */
    void update( const void*, unsigned long )
    {
        using namespace kjb_c;
        SET_CANT_HAPPEN_BUG();
    }
#endif

    /* We decline to make this copyable, because then we would need to store
     * the algorithm selection as an extra field, and we don't want to.
     */
    OpenSSL_EVP( const OpenSSL_EVP& );              // copy ctor teaser
    OpenSSL_EVP& operator=( const OpenSSL_EVP& );   // assignment teaser

public:
    /**
     * @brief ctor must specify the algorithm
     *
     * OpenSSL provides helper functions to return the appropriate algorithm
     * pointer, such as EVP_md5() and EVP_sha1().
     *
     * The end user is encouraged to create a derived class for a specific
     * algorithm:  see class MD5 for an example.
     */
#ifdef KJB_HAVE_OPENSSL
    OpenSSL_EVP( const EVP_MD* algorithm )
    :   m_digest( EVP_MAX_MD_SIZE ), // strategy:  allocate extra, shrink later
        m_finalized( false )
    {
        EVP_MD_CTX_init( &m_context );
        ETX( 0 == EVP_DigestInit_ex( &m_context, algorithm, 00 ) );
        m_digest.resize( EVP_MD_CTX_size( &m_context ) );// it is later; shrink
        // I want to do all the memory allocation for m_digest at its ctor time
    }
#else
    OpenSSL_EVP()
    :   m_finalized( true )
    {
        KJB_THROW_2(Missing_dependency,
                "Missing OpenSSL -- cannot instantiate kjb::OpenSSL_EVP");
    }
#endif

    /// @brief this thing needs a dtor or it leaks
    ~OpenSSL_EVP()
    {
#ifdef KJB_HAVE_OPENSSL
        EVP_MD_CTX_cleanup( &m_context );
#endif
    }

    /**
     * @brief Hash an array of bytes; len is the number of bytes
     *
     * This is the old-skool C paradigm for writing stuff.
     */
    void write( const void* data, unsigned long len )
    {
        // update will call you out if the object is already finalized
        update( data, len );
    }

    /// @brief Hash a single primitive datum, using a "shallow read"
    template< typename T >
    void put( const T& datum )
    {
        write( (const void*) & datum, sizeof( T ) );
    }

    /// @brief Hash a container of primitive data via STL sequential iterators
    template< typename SeqIterator >
    void write( SeqIterator begin, SeqIterator end )
    {
        // I wanted to use std::for_each but I couldn't build the req. functor.
        // To me it was trickier than at first it looked, b/c put is a template
        for( SeqIterator ppp = begin; ppp != end; )
            put( *ppp++ );
    }

    // Digest-getter methods =================================

    /// @brief Return size of message digest
    size_t size() const
    {
        return m_digest.size();
    }

    /// @brief Return a copy of the message digest
    Digest copy_digest()
    {
        safe_finalize();
        return m_digest; // return by value is OK because it is not very big
    }

    /// @brief Access one character of the message digest
    Unit operator[]( size_t index )
    {
        safe_finalize();
        return m_digest.at( index );
    }

    /// @brief Return const iterator to start of message digest
    Digest::const_iterator begin()
    {
        safe_finalize();
        return m_digest.begin();
    }

    /// @brief Return const iterator to one-past-last of message digest
    Digest::const_iterator end()
    {
        safe_finalize(); // adds to the cost a bit but makes the class safer
        return m_digest.end();
    }

    /// @brief Return whether the hasher is finalized, i.e., can I write to it?
    bool is_finalized() const
    {
        return m_finalized;
    }
};


} // end namespace kjb

#endif

