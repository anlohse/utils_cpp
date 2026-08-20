/*
 * utils_config.hpp
 *
 *  Created on: 24/09/2009
 *      Author: alanlohse
 */

#ifndef UTILS_CONFIG_HPP_
#define UTILS_CONFIG_HPP_

#include <cstdio>
#include <cstdlib>

// These may already have been supplied by the build system, which is the
// preferred source of truth; the #ifndef guards keep an -D on the command line
// from colliding with the fallback detection below.
#if defined(_WIN32) || defined(__WIN32) || defined(_WIN32_)
#  ifndef UTILS_WINDOWS
#    define UTILS_WINDOWS
#  endif
#  if defined(USE_PTHREAD)
#    ifndef PTHREAD_IMPL
#      define PTHREAD_IMPL
#    endif
#  else
#    ifndef WINDOWS_THREAD_IMPL
#      define WINDOWS_THREAD_IMPL
#    endif
#  endif
#endif

#if defined(_LINUX) || defined(__LINUX) || defined(_LINUX_)
#  ifndef UTILS_LINUX
#    define UTILS_LINUX
#  endif
#  ifndef PTHREAD_IMPL
#    define PTHREAD_IMPL
#  endif
#include "linux.inc"
#endif

#endif /* UTILS_CONFIG_HPP_ */
