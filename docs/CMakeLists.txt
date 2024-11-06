

message(STATUS "Generating Doxygen documentation")

doxygen_add_docs(docs
                 CONFIG_FILE "${CMAKE_CURRENT_SOURCE_DIR}/doxygen.conf"
                 WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}")