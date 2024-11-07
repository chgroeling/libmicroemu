if (CMAKE_PARTS MATCHES "Visual Studio")
    # For Windows: Prevent overriding the parent project's compiler/linker settings
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
endif()

enable_testing()

find_package(GTest REQUIRED COMPONENTS gtest gmock gtest_main gmock_main)
include(GoogleTest)

macro(package_add_test TESTNAME)
    # create an executable in which the tests will be stored
    add_executable(${TESTNAME} ${ARGN})
    # link the Google test infrastructure, mocking library, and a default main function to
    # the test executable.  Remove g_test_main if writing your own main function.
    target_link_libraries(${TESTNAME}
        GTest::gtest
        GTest::gmock
        GTest::gtest_main
        GTest::gmock_main
        libmicroemu
    )
    target_include_directories(${TESTNAME} PRIVATE ../src)

    # gtest_discover_tests replaces gtest_add_tests,
    # see https://cmake.org/cmake/help/v3.10/module/GoogleTest.html for more options to pass to it
    gtest_discover_tests(${TESTNAME}
        # set a working directory so your project root so that you can find test data via paths relative to the project root
        WORKING_DIRECTORY ${PROJECT_DIR}
        PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${PROJECT_DIR}"
    )

    set_target_properties(${TESTNAME} PROPERTIES FOLDER tests)
endmacro()

set(TEST_SOURCES
    test_microemu.cpp
    microemu/internal/endianess_converters_test.cpp
    microemu/utils/bit_manip_tests.cpp
    microemu/utils/alu_tests.cpp
) 

package_add_test(ptest ${TEST_SOURCES})

set(SYSTEST_SCRIPT python ${CMAKE_SOURCE_DIR}/tests/system_tests/system_test.py)
set(SYSTEST_INPUT_DIR ${CMAKE_SOURCE_DIR}/tests/system_tests/)
set(SYSTEST_OUTPUT_DIR ${CMAKE_BINARY_DIR}/system_tests/)

add_test(
    NAME SystemTests.bare_stdlib 
    COMMAND ${SYSTEST_SCRIPT} $<TARGET_FILE:microemu> "-m STDLIB" ${SYSTEST_INPUT_DIR} ${SYSTEST_OUTPUT_DIR} bare_stdlib
)

add_test(
    NAME SystemTests.bare_stdlib_nano 
    COMMAND ${SYSTEST_SCRIPT} $<TARGET_FILE:microemu> "-m STDLIB" ${SYSTEST_INPUT_DIR} ${SYSTEST_OUTPUT_DIR} bare_stdlib_nano
)

add_test(
    NAME SystemTests.printf_rdimon 
    COMMAND ${SYSTEST_SCRIPT} $<TARGET_FILE:microemu> "-m MINIMAL" ${SYSTEST_INPUT_DIR} ${SYSTEST_OUTPUT_DIR} printf_rdimon
)

