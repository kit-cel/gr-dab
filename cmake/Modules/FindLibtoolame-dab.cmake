# Try to find the libtoolame-dab source library.
# Once done this will define
#
# LIBTOOLAME-DAB_SOURCE_DIRS - where to find toolame.h, etc.

find_path(LIBTOOLAME-DAB_SOURCE_DIR libtoolame-dab/toolame.h
  ${CMAKE_SOURCE_DIR}/ODR-AudioEnc
  /opt/local/include
  /usr/local/include
  /usr/local/include/libtoolame-dab
  /usr/include
  /usr/include/libtoolame-dab
)

if(LIBTOOLAME-DAB_SOURCE_DIR)
  set(LIBTOOLAME-DAB_INCLUDE_DIR "${LIBTOOLAME-DAB_SOURCE_DIR}")
  set(LIBTOOLAME-DAB_SOURCE_DIR "${LIBTOOLAME-DAB_SOURCE_DIR}/libtoolame-dab")
  set(LIBTOOLAME-DAB_FOUND 1)
endif(LIBTOOLAME-DAB_SOURCE_DIR)


mark_as_advanced(LIBTOOLAME-DAB_SOURCE_DIR)
mark_as_advanced(LIBTOOLAME-DAB_INCLUDE_DIR)
mark_as_advanced(LIBTOOLAME-DAB_FOUND)

if(NOT LIBTOOLAME-DAB_FOUND)
  set(LIBTOOLAME-DAB_DIR_MESSAGE "libtoolame-dab was not found. Make sure and LIBTOOLAME-DAB_SOURCE_DIR is set.")
  if(NOT LIBTOOLAME-DAB_FIND_QUIETLY)
    message(STATUS "${LIBTOOLAME-DAB_DIR_MESSAGE}")
  else(NOT LIBTOOLAME-DAB_FIND_QUIETLY)
    if(LIBTOOLAME-DAB_FIND_REQUIRED)
      message(FATAL_ERROR "${LIBTOOLAME-DAB_DIR_MESSAGE}")
    endif(LIBTOOLAME-DAB_FIND_REQUIRED)
  endif(NOT LIBTOOLAME-DAB_FIND_QUIETLY)
endif(NOT LIBTOOLAME-DAB_FOUND)
