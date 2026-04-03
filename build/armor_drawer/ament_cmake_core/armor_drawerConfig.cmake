# generated from ament/cmake/core/templates/nameConfig.cmake.in

# prevent multiple inclusion
if(_armor_drawer_CONFIG_INCLUDED)
  # ensure to keep the found flag the same
  if(NOT DEFINED armor_drawer_FOUND)
    # explicitly set it to FALSE, otherwise CMake will set it to TRUE
    set(armor_drawer_FOUND FALSE)
  elseif(NOT armor_drawer_FOUND)
    # use separate condition to avoid uninitialized variable warning
    set(armor_drawer_FOUND FALSE)
  endif()
  return()
endif()
set(_armor_drawer_CONFIG_INCLUDED TRUE)

# output package information
if(NOT armor_drawer_FIND_QUIETLY)
  message(STATUS "Found armor_drawer: 1.1.4 (${armor_drawer_DIR})")
endif()

# warn when using a deprecated package
if(NOT "" STREQUAL "")
  set(_msg "Package 'armor_drawer' is deprecated")
  # append custom deprecation text if available
  if(NOT "" STREQUAL "TRUE")
    set(_msg "${_msg} ()")
  endif()
  # optionally quiet the deprecation message
  if(NOT ${armor_drawer_DEPRECATED_QUIET})
    message(DEPRECATION "${_msg}")
  endif()
endif()

# flag package as ament-based to distinguish it after being find_package()-ed
set(armor_drawer_FOUND_AMENT_PACKAGE TRUE)

# include all config extra files
set(_extras "")
foreach(_extra ${_extras})
  include("${armor_drawer_DIR}/${_extra}")
endforeach()
