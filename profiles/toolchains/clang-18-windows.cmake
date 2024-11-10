# Windows has only one installation of clang. In our case, it is clang-18.
# compiler/ linker programs
set(CMAKE_C_COMPILER   clang${CMAKE_EXECUTABLE_SUFFIX} )
set(CMAKE_CXX_COMPILER clang++${CMAKE_EXECUTABLE_SUFFIX} )
set(CMAKE_ASM_COMPILER clang${CMAKE_EXECUTABLE_SUFFIX})
set(CMAKE_OBJCOPY      llvm-objcopy${CMAKE_EXECUTABLE_SUFFIX} CACHE INTERNAL "objcopy tool")
set(CMAKE_OBJDUMP      llvm-objdump${CMAKE_EXECUTABLE_SUFFIX} CACHE INTERNAL "objdump tool")
set(CMAKE_OBJSIZE      llvm-size${CMAKE_EXECUTABLE_SUFFIX} CACHE INTERNAL "size tool")

