
// Author(s): Efi Fogel         <efifogel@gmail.com>

#ifndef FDMLPY_TO_STRING_HPP
#define FDMLPY_TO_STRING_HPP

#include <string>
#include <sstream>

template <typename T>
std::string to_string(const T& n) {
  std::ostringstream os;
  os << n;
  return os.str();
}

#endif
