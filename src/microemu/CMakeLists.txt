
set(APP_NAME microemu)
set(APP_SOURCES
  main.cpp
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

add_executable(${APP_NAME} ${APP_SOURCES})

target_compile_options(${APP_NAME} PUBLIC "$<$<CONFIG:DEBUG>:${DEBUG_COMPILE_OPTIONS}>")
target_compile_options(${APP_NAME} PUBLIC "$<$<CONFIG:RELEASE>:${RELEASE_COMPILE_OPTIONS}>")

# Move the output to the bin directory
set_target_properties(${APP_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

# Link local libraries
target_link_libraries(${APP_NAME} PRIVATE libmicroemu)

# ------------------------------------
# FMT
# ------------------------------------
find_package(fmt)
target_link_libraries(${APP_NAME} PRIVATE fmt::fmt)

# ------------------------------------
# spdlog
# ------------------------------------
find_package(spdlog)
target_link_libraries(${APP_NAME} PRIVATE spdlog::spdlog)

# ------------------------------------
# CXXOPTS
# ------------------------------------
find_package(cxxopts)
target_link_libraries(${APP_NAME} PRIVATE cxxopts::cxxopts)


install(TARGETS ${APP_NAME} DESTINATION bin)
install(FILES ../../licenses/microemu/THIRD_PARTY_LICENSES DESTINATION bin/)
install(FILES ../../LICENSE DESTINATION bin/)
install(FILES ../../VERSION DESTINATION bin/)