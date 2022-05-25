// Copyright (c) 2022 Israel.
// All rights reserved.
//
// Author(s): Efi Fogel         <efifogel@gmail.com>

#ifndef FDML_CONFIG_HPP
#define FDML_CONFIG_HPP

#include <boost/config.hpp>

#include "fdml/version.hpp"

///////////////////////////////////////////////////////////////////////////////
// Windows DLL suport
#ifdef BOOST_HAS_DECLSPEC
#if defined(FDML_ALL_DYN_LINK) || defined(FDML_FDML_DYN_LINK)
// export if this is our own source, otherwise import:
#ifdef FDML_FDML_SOURCE
#define FDML_FDML_DECL __declspec(dllexport)
#else
#define FDML_FDML_DECL __declspec(dllimport)
#endif // FDML_FDML_SOURCE
#endif // DYN_LINK
#endif // FDML_HAS_DECLSPEC

#ifndef FDML_FDML_DECL
#define FDML_FDML_DECL
#endif

#endif
