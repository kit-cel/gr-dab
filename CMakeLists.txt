# Copyright 2011,2012 Free Software Foundation, Inc.
#
# This file is part of GNU Radio
#
# GNU Radio is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3, or (at your option)
# any later version.
#
# GNU Radio is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with GNU Radio; see the file COPYING.  If not, write to
# the Free Software Foundation, Inc., 51 Franklin Street,
# Boston, MA 02110-1301, USA.


########################################################################
# Project setup
########################################################################
cmake_minimum_required(VERSION 3.8)
project(gr-dab CXX C)
enable_testing()

cmake_policy(SET CMP0011 NEW)

#select the release build type by default to get optimization flags
if(NOT CMAKE_BUILD_TYPE)
   set(CMAKE_BUILD_TYPE "Release")
   message(STATUS "Build type not specified: defaulting to release.")
endif(NOT CMAKE_BUILD_TYPE)
set(CMAKE_BUILD_TYPE ${CMAKE_BUILD_TYPE} CACHE STRING "")

list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/Modules)

########################################################################
# Compiler specific setup
########################################################################
if(CMAKE_COMPILER_IS_GNUCXX AND NOT WIN32)
    #http://gcc.gnu.org/wiki/Visibility
    add_definitions(-fvisibility=hidden)
endif()

########################################################################
# Find FAAD
########################################################################
find_package(Faad REQUIRED)
if (NOT FAAD_FOUND )
    message(FATAL_ERROR "please install libfaad")
else()
    message(STATUS "FAAD found")
endif ()

########################################################################
# Find FDK-AAC-DAB
########################################################################
find_package(Fdk-aac-dab REQUIRED)
if (NOT FDK-AAC-DAB_FOUND )
    message(FATAL_ERROR "please install fdk-aac-dab:
You'll find it in the eponymous subdirectory; build it using:

    $ cd fdk-aac-dab
    $ ./bootstrap
    $ ./configure [--prefix ...] [--other options]
    $ make
    $ make install

")
else()
    message(STATUS "FDK-AAC-DAB found")
endif ()

########################################################################
# Find LIBTOOLAME-DAB
########################################################################
find_package(Libtoolame-dab REQUIRED)
if (NOT LIBTOOLAME-DAB_FOUND )
    message(FATAL_ERROR "please make sure the libtoolame-dab source can be found")
else()
    message(STATUS "LIBTOOLAME-DAB found: ${LIBTOOLAME-DAB_SOURCE_DIR}")
  endif ()

find_package(Gnuradio "3.8" REQUIRED COMPONENTS filter fft blocks)

include(GrPython)
# Find PyQt5
find_program(PYUIC5_EXECUTABLE pyuic5)
########################################################################
# Install directories
########################################################################
include(GrPlatform) #define LIB_SUFFIX
set(GR_RUNTIME_DIR      bin)
set(GR_LIBRARY_DIR      lib${LIB_SUFFIX})
set(GR_INCLUDE_DIR      include/dab)
set(GR_DATA_DIR         share)
set(GR_PKG_DATA_DIR     ${GR_DATA_DIR}/${CMAKE_PROJECT_NAME})
set(GR_DOC_DIR          ${GR_DATA_DIR}/doc)
set(GR_PKG_DOC_DIR      ${GR_DOC_DIR}/${CMAKE_PROJECT_NAME})
set(GR_CONF_DIR         etc)
set(GR_PKG_CONF_DIR     ${GR_CONF_DIR}/${CMAKE_PROJECT_NAME}/conf.d)
set(GR_LIBEXEC_DIR      libexec)
set(GR_PKG_LIBEXEC_DIR  ${GR_LIBEXEC_DIR}/${CMAKE_PROJECT_NAME})
set(GRC_BLOCKS_DIR      ${GR_PKG_DATA_DIR}/grc/blocks)

########################################################################
# Find gnuradio build dependencies
########################################################################
find_package(Doxygen)


########################################################################
# Setup doxygen option
########################################################################
if(DOXYGEN_FOUND)
	option(ENABLE_DOXYGEN "Build docs using Doxygen" ON)
else(DOXYGEN_FOUND)
	option(ENABLE_DOXYGEN "Build docs using Doxygen" OFF)
endif(DOXYGEN_FOUND)

########################################################################
# Create uninstall target
########################################################################
configure_file(
    ${CMAKE_SOURCE_DIR}/cmake/cmake_uninstall.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake
@ONLY)

add_custom_target(uninstall
    ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake
)

########################################################################
# Add subdirectories
########################################################################
add_subdirectory(include/dab)
add_subdirectory(lib)
add_subdirectory(swig)
add_subdirectory(python)
add_subdirectory(python/GUI)
add_subdirectory(grc)
add_subdirectory(apps)
add_subdirectory(docs)

########################################################################
# Install cmake search helper for this library
########################################################################
install(FILES cmake/Modules/dabConfig.cmake
    DESTINATION lib/cmake/dab
)
