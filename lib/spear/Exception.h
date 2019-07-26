#ifndef EXCEPTION_H_
#define EXCEPTION_H_

/*!
 * @file Exception.h
 *
 * @author Mihai Surdeanu
 * $Id: Exception.h 16330 2014-02-04 18:29:14Z cdawson $ 
 */

#include <string>

namespace spear {

class Exception
{
 public:

  Exception(const std::string & msg) : _message(msg) {};

  Exception(const std::string & msg, int line);

  const std::string & getMessage() const { return _message; };

 private:
  
  std::string _message;
};

} // end namespace spear

#endif
