# Try to find fdk-aac library and include path.
# Once done this will define
#
# FDK-AAC_INCLUDE_DIRS - where to find fdk-aac.h, etc.
# FDK-AAC_LIBRARIES - List of libraries when using libfdk-aac.
# FDK-AAC_FOUND - True if libfdk-aac found.

find_path(FDK-AAC_INCLUDE_DIR fdk-aac/aacenc_lib.h
  /opt/local/include
  /usr/local/include
  /usr/include
)
find_library(FDK-AAC_LIBRARY NAMES fdk-aac DOC "The libfdk-aac library")

if(FDK-AAC_INCLUDE_DIR AND FDK-AAC_LIBRARY)
  set(FDK-AAC_FOUND 1)
  set(FDK-AAC_LIBRARIES ${FDK-AAC_LIBRARY})
  set(FDK-AAC_INCLUDE_DIRS ${FDK-AAC_INCLUDE_DIR})
else(FDK-AAC_INCLUDE_DIR AND FDK-AAC_LIBRARY)
  set(FDK-AAC_FOUND 0)
  set(FDK-AAC_LIBRARIES)
  set(FDK-AAC_INCLUDE_DIRS)
endif(FDK-AAC_INCLUDE_DIR AND FDK-AAC_LIBRARY)

mark_as_advanced(FDK-AAC_INCLUDE_DIR)
mark_as_advanced(FDK-AAC_LIBRARY)
mark_as_advanced(FDK-AAC_FOUND)

if(NOT FDK-AAC_FOUND)
  set(FDK-AAC_DIR_MESSAGE "libfdk-aac was not found. Make sure FDK-AAC_LIBRARY and FDK-AAC_INCLUDE_DIR are set.")
  if(NOT FDK-AAC_FIND_QUIETLY)
    message(STATUS "${FDK-AAC_DIR_MESSAGE}")
  else(NOT FDK-AAC_FIND_QUIETLY)
    if(FDK-AAC_FIND_REQUIRED)
      message(FATAL_ERROR "${FDK-AAC_DIR_MESSAGE}")
    endif(FDK-AAC_FIND_REQUIRED)
  endif(NOT FDK-AAC_FIND_QUIETLY)
endif(NOT FDK-AAC_FOUND)
