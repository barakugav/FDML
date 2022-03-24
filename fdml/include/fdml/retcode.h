#ifndef FDML_RETCODE_H
#define FDML_RETCODE_H

namespace FDML {

enum fdml_retcode {
  FDML_RETCODE_OK = 0,
  FDML_RETCODE_MISSING_ARGS = 100,
  FDML_RETCODE_UNKNOWN_ARGS = 101,
  FDML_RETCODE_TOO_MANY_ARGS = 102,
  FDML_RETCODE_RUNTIME_ERR = 200,
  FDML_RETCODE_INTERNAL_ERR = 300,
};

} // namespace FDML

#endif
