cmake_minimum_required (VERSION 3.5.0)
message(STATUS "CMake version: ${CMAKE_VERSION}")

# Read the version from VERSION file
file(READ "${CMAKE_SOURCE_DIR}/VERSION" PROJECT_VERSION)

# Remove any surrounding whitespace
string(STRIP "${PROJECT_VERSION}" PROJECT_VERSION)  

# Remove leading 'v' if present
string(REGEX REPLACE "^v" "" PROJECT_VERSION "${PROJECT_VERSION}")

# Split the version into major, minor, and patch components
string(REPLACE "." ";" VERSION_LIST "${PROJECT_VERSION}")
list(GET VERSION_LIST 0 PROJECT_VERSION_MAJOR)
list(GET VERSION_LIST 1 PROJECT_VERSION_MINOR)
list(GET VERSION_LIST 2 PROJECT_VERSION_PATCH)

# Combine to form the SEMVER format
set(PROJECT_VERSION "${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}")

# Project name and a few useful settings. Other commands can pick up the results
project(
  libmicroemu 
  VERSION ${PROJECT_VERSION}
  DESCRIPTION "Embeddedable emulator on microcontrollers"
  LANGUAGES CXX C)

# Confirm version
message(STATUS "libmicroemu version: ${PROJECT_VERSION}")



# Only do these if this is the main project, and not if it is included through add_subdirectory
if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
  set(CMAKE_CXX_STANDARD 17)
  set(CMAKE_CXX_STANDARD_REQUIRED TRUE)
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
