# Try to find fdk-aac-dab library and include path.
# Once done this will define
#
# FDK-AAC-DAB_INCLUDE_DIRS - where to find fdk-aac-dab.h, etc.
# FDK-AAC-DAB_LIBRARIES - List of libraries when using libfdk-aac-dab.
# FDK-AAC-DAB_FOUND - True if libfdk-aac-dab found.

find_path(FDK-AAC-DAB_INCLUDE_DIR fdk-aac-dab/aacenc_lib.h
  /opt/local/include
  /usr/local/include
  /usr/include
)
find_library(FDK-AAC-DAB_LIBRARY NAMES fdk-aac-dab DOC "The libfdk-aac-dab library")

if(FDK-AAC-DAB_INCLUDE_DIR AND FDK-AAC-DAB_LIBRARY)
  set(FDK-AAC-DAB_FOUND 1)
  set(FDK-AAC-DAB_LIBRARIES ${FDK-AAC-DAB_LIBRARY})
  set(FDK-AAC-DAB_INCLUDE_DIRS ${FDK-AAC-DAB_INCLUDE_DIR})
else(FDK-AAC-DAB_INCLUDE_DIR AND FDK-AAC-DAB_LIBRARY)
  set(FDK-AAC-DAB_FOUND 0)
  set(FDK-AAC-DAB_LIBRARIES)
  set(FDK-AAC-DAB_INCLUDE_DIRS)
endif(FDK-AAC-DAB_INCLUDE_DIR AND FDK-AAC-DAB_LIBRARY)

mark_as_advanced(FDK-AAC-DAB_INCLUDE_DIR)
mark_as_advanced(FDK-AAC-DAB_LIBRARY)
mark_as_advanced(FDK-AAC-DAB_FOUND)

if(NOT FDK-AAC-DAB_FOUND)
  set(FDK-AAC-DAB_DIR_MESSAGE "libfdk-aac-dab was not found. Make sure FDK-AAC-DAB_LIBRARY and FDK-AAC-DAB_INCLUDE_DIR are set.")
  if(NOT FDK-AAC-DAB_FIND_QUIETLY)
    message(STATUS "${FDK-AAC-DAB_DIR_MESSAGE}")
  else(NOT FDK-AAC-DAB_FIND_QUIETLY)
    if(FDK-AAC-DAB_FIND_REQUIRED)
      message(FATAL_ERROR "${FDK-AAC-DAB_DIR_MESSAGE}")
    endif(FDK-AAC-DAB_FIND_REQUIRED)
  endif(NOT FDK-AAC-DAB_FIND_QUIETLY)
endif(NOT FDK-AAC-DAB_FOUND)
