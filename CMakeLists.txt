cmake_minimum_required (VERSION 3.5.0)

message("Building with CMake version: ${CMAKE_VERSION}")

#include(cmake/print_target_properties.cmake)

# Project name and a few useful settings. Other commands can pick up the results
project(
  libmicroemu 
  VERSION 0.0.1
  DESCRIPTION "Embeddedable emulator on microcontrollers"
  LANGUAGES CXX C)


# Only do these if this is the main project, and not if it is included through add_subdirectory
if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
  set(CMAKE_CXX_STANDARD 17)
  set(CMAKE_CXX_STANDARD_REQUIRED True)
  set(CMAKE_CXX_EXTENSIONS FALSE) 
  
  # Docs only available if this is the main app
  find_package(Doxygen)
  if(Doxygen_FOUND)
    add_subdirectory(docs)
  else()
    message(STATUS "Doxygen not found, not building docs")
  endif()
endif()


# ------------------------------------
# Library: libmicroemu
# ------------------------------------
add_subdirectory(src/libmicroemu)

# ------------------------------------
# Application: microemu 
# ------------------------------------
if (BUILD_MICROEMU)
    add_subdirectory(src/microemu)
endif()

# Testing only available if this is the main app
if ((CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME) AND BUILD_TESTING AND BUILD_MICROEMU)
  # Testing only available if this is the main app
  # Note this needs to be done in the main CMakeLists
  # since it calls enable_testing, which must be in the
  # main CMakeLists.
  include(CTest)

  add_subdirectory(tests)
endif()
