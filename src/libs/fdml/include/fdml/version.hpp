// Copyright (c) 2022 Israel.
// All rights reserved.
//
// Author(s): Efi Fogel         <efifogel@gmail.com>

#ifndef FDML_VERSION_HPP
#define FDML_VERSION_HPP

/*! \file
 * Caution, this is the only FDML header that is guarenteed
 * to change with every FDML release, including this header
 * will cause a recompile every time a new FDML version is
 * released.
 *
 * FDML_VERSION % 100 is the sub-minor version
 * FDML_VERSION / 100 % 1000 is the minor version
 * FDML_VERSION / 100000 is the major version
 */

#define FDML_VERSION 100001

//
//  FDML_LIB_VERSION must be defined to be the same as FDML_VERSION
//  but as a *string* in the form "x_y" where x is the major version
//  number and y is the minor version number.  This is used by
//  <config/auto_link.hpp> to select which library version to link to.

#define FDML_LIB_VERSION "1_01"

#endif
