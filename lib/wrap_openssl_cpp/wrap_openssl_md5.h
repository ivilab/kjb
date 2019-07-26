/**
 * @file
 * @author Andrew Predoehl
 * @brief Wrapper for MD5 (message digest 5) hash provided by OpenSSL
 *
 * Wrapped so it will still compile even with NO_LIBS.
 *
 * Please see class kjb::MD5.
 */
/*
 * $Id: wrap_openssl_md5.h 17473 2014-09-08 01:22:34Z predoehl $
 */

#ifndef OPENSSL_CPP_MD5_H_WRAP_PREDOEHL_UOFARIZONA_VISION
#define OPENSSL_CPP_MD5_H_WRAP_PREDOEHL_UOFARIZONA_VISION

#include <wrap_openssl_cpp/wrap_openssl_evp.h>

namespace kjb
{

/**
 * @brief Message Digest 5 computation, using OpenSSL code
 *
 * @throws kjb::Missing_dependency if compiled with NO_LIBS or missing OpenSSL
 */
class MD5
:   public OpenSSL_EVP
{
public:
#ifdef KJB_HAVE_OPENSSL
    MD5()
    :   OpenSSL_EVP( EVP_md5() )
    {}
#endif
};


} // end namespace kjb

#endif

