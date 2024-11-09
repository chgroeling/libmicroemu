
message(STATUS "Adding target \"docs\" to generate Doxygen documentation")


set(DOXYGEN_CONFIG_FILE "${CMAKE_BINARY_DIR}/doxygen.conf")

# Generate version.h in the build directory
configure_file(
    "${PROJECT_SOURCE_DIR}/cmake/doxygen.conf.in"    
    "${DOXYGEN_CONFIG_FILE}"
    @ONLY
)


add_custom_target(docs
    COMMAND "${DOXYGEN_EXECUTABLE}" "${DOXYGEN_CONFIG_FILE}"
    WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}"
    COMMENT "Generating API documentation with Doxygen"
    VERBATIM
)