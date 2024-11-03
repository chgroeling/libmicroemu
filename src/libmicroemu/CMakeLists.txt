
set(LIB_NAME libmicroemu)

# Create the library 
set (MICROEMU_SOURCES 
  microemu.cpp
  logger.cpp
)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
SET (
  COMPILER_WARNING_SWITCHES
  "-Wall"
  "-Wextra"
  "-Wno-comment"
  "-Wno-parentheses"
  "-Wno-format-zero-length"
  "-Wpedantic"
  "-pedantic"
#  "-Wconstexpr-not-const"
)

SET (
  DEBUG_COMPILE_OPTIONS
  "-O0"
  "-g"
  "-DDEBUG"
)

SET (
  RELEASE_COMPILE_OPTIONS
  "-O3"
  "-DNDEBUG"
)

SET (DEBUG_LOGLEVEL 
    "LOGLEVEL_TRACE"
)

SET (RELEASE_LOGLEVEL 
    "LOGLEVEL_INFO"
)
add_library(${LIB_NAME} ${MICROEMU_SOURCES})

set_target_properties(${LIB_NAME} PROPERTIES PREFIX "")
target_compile_definitions(${LIB_NAME} PUBLIC "$<$<CONFIG:DEBUG>:${DEBUG_LOGLEVEL}>")
target_compile_definitions(${LIB_NAME} PUBLIC "$<$<CONFIG:RELEASE>:${RELEASE_LOGLEVEL}>")
target_compile_options(${LIB_NAME}
    PRIVATE
    ${COMPILER_WARNING_SWITCHES}
)

target_compile_options(${LIB_NAME} PUBLIC "$<$<CONFIG:DEBUG>:${DEBUG_COMPILE_OPTIONS}>")
target_compile_options(${LIB_NAME} PUBLIC "$<$<CONFIG:RELEASE>:${RELEASE_COMPILE_OPTIONS}>")

# Add the include directories and make them known to other targets (PUBLIC)
target_include_directories(
    ${LIB_NAME} 
    PUBLIC 
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../include>  
    $<INSTALL_INTERFACE:include>  # <prefix>/include/
    PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src
)

# Install the public headers
install(DIRECTORY ../../include DESTINATION include
)

install(
    TARGETS ${LIB_NAME} 
    EXPORT ${LIB_NAME}-targets
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
    INCLUDES DESTINATION include
)

install(EXPORT ${LIB_NAME}-targets
    FILE ${LIB_NAME}-targets.cmake
    NAMESPACE ${LIB_NAME}::
    DESTINATION share/${LIB_NAME}/cmake
)

install(FILES ../../LICENSE DESTINATION share/${LIB_NAME})
