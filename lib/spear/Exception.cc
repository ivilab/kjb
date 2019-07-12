/*!
 * @file Exception.cc
 *
 * @author Mihai Surdeanu
 * $Id$ 
 */

#include <sstream>

#include "spear/Exception.h"

using namespace std;
using namespace spear;

Exception::Exception(const std::string & msg, int line)
{
  ostringstream buffer;
  buffer << "[Line " << line << "] " << msg << ends;
  _message = buffer.str();
}
