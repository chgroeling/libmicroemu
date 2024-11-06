
message(STATUS "Adding target \"docs\" to generate Doxygen documentation")

set(DOXYGEN_CONFIG_FILE "${CMAKE_CURRENT_SOURCE_DIR}/doxygen.conf")

add_custom_target(docs
    COMMAND "${DOXYGEN_EXECUTABLE}" "${DOXYGEN_CONFIG_FILE}"
    WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}"
    COMMENT "Generating API documentation with Doxygen"
    VERBATIM
)