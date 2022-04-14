// Copyright (c) 2022 Israel.
// All rights reserved to Tel Aviv University.
//
// Author(s): Efi Fogel         <efifogel@gmail.com>

#ifndef FDML_BINDINGS_TYPES_HPP
#define FDML_BINDINGS_TYPES_HPP

#include <boost/python.hpp>

namespace bp = boost::python;

typedef bp::return_value_policy<bp::copy_const_reference> Copy_const_reference;
typedef bp::return_value_policy<bp::return_by_value>      Return_by_value;
typedef bp::return_value_policy<bp::manage_new_object>    Manage_new_object;
typedef bp::return_value_policy<bp::reference_existing_object>
  Reference_existing_object;
typedef bp::return_value_policy<bp::copy_non_const_reference>
  Copy_non_const_reference;

typedef Return_by_value                                   Kernel_return_value_policy;

#endif
