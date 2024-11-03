set(DOXYGEN_EXTRACT_ALL YES)
set(DOXYGEN_BUILTIN_STL_SUPPORT YES)

set(DOCS_SOURCES
  bitstream/bitstream.h
  "${CMAKE_CURRENT_SOURCE_DIR}/mainpage.md"
)
doxygen_add_docs(docs ${DOCS_SOURCES}
                 WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}/include")