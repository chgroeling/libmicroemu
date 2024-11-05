
cmake_minimum_required (VERSION 3.5.0)

# Project name and a few useful settings. Other commands can pick up the results
project(
  printf_rdimon 
  VERSION 0.0.1
  DESCRIPTION "Embeddedable emulator on microcontrollers"
  LANGUAGES CXX C ASM
)

set(TARGET_NAME printf_rdimon)

set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_CXX_STANDARD 14)

# Create the library 
set (TARGET_SOURCES 
  main.cpp
  syscalls.c
  startup.s
)

add_executable(${TARGET_NAME} ${TARGET_SOURCES})

set_target_properties(${TARGET_NAME}  PROPERTIES SUFFIX ".elf")
set_target_properties(${TARGET_NAME}  PROPERTIES
    LINK_FLAGS " -T ${CMAKE_SOURCE_DIR}/linker.ld -Wl,-Map=${CMAKE_BINARY_DIR}/${TARGET_NAME}.map"
)


# Execute objdump to extract dissambly from compiled program
add_custom_command(
    TARGET ${TARGET_NAME} 
    POST_BUILD
    COMMAND ${CMAKE_OBJDUMP} -d -S -C --no-recurse-limit  "$<TARGET_FILE:${TARGET_NAME}>" > "disassembly"
)

add_custom_command(
    TARGET ${TARGET_NAME} 
    POST_BUILD
    COMMAND ${CMAKE_OBJDUMP} -s -C --no-recurse-limit "$<TARGET_FILE:${TARGET_NAME}>" > "full_contents"
)

add_custom_command(
    TARGET ${TARGET_NAME} 
    POST_BUILD
    COMMAND arm-none-eabi-readelf --segments -W -C --no-recurse-limit "$<TARGET_FILE:${TARGET_NAME}>" > "segments"
)

add_custom_command(
    TARGET ${TARGET_NAME} 
    POST_BUILD
    COMMAND arm-none-eabi-readelf --sections -W -C --no-recurse-limit "$<TARGET_FILE:${TARGET_NAME}>" > "sections"
)

add_custom_command(
    TARGET ${TARGET_NAME} 
    POST_BUILD
    COMMAND arm-none-eabi-readelf -s -W -C --no-recurse-limit "$<TARGET_FILE:${TARGET_NAME}>" > "symbols"
)

install(TARGETS ${TARGET_NAME}  DESTINATION bin)
install(FILES ${CMAKE_BINARY_DIR}/${TARGET_NAME}.map DESTINATION infos)
install(FILES ${CMAKE_BINARY_DIR}/sections DESTINATION infos)
install(FILES ${CMAKE_BINARY_DIR}/segments DESTINATION infos)
install(FILES ${CMAKE_BINARY_DIR}/symbols DESTINATION infos)
install(FILES ${CMAKE_BINARY_DIR}/disassembly DESTINATION infos)