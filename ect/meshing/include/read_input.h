#ifndef READ_INPUT_H_
#define READ_INPUT_H_

#include "cgal_include.h"
#include <string>

// Reads a *.poly file to a CGAL arrangement
// The format of a *.poly file (specific to the FDML project):
//      x1 y1\n
//      x2 y2\n
//       ...
// First and last vertices must *not* be equal, and in general
// zero length edges are forbidden. 
void load_poly_to_arrangement(std::string& filename, Arrangement* arr);

#endif
