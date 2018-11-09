cmake_minimum_required(VERSION 3.6.0)

project(mg_ray)      


#enable c++ 17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

#check the compiler
MESSAGE(STATUS "compiler id ${CMAKE_CXX_COMPILER_ID} ${MSVC}")
if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
  # using Clang
  set(COMMON_CXX_FLAGS "${COMMON_CXX_FLAGS} -g3 -O0 -Wall -pedantic -Wextra -m64 -mavx2 -mfma -ffast-math")
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
  # using GCC
  set(COMMON_CXX_FLAGS "${COMMON_CXX_FLAGS}   -Wall -pedantic -Wextra -m64 -mavx2 -mfma -ffast-math")
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Intel")
  # using Intel C++
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
  # using Visual Studio C++
  set(COMMON_CXX_FLAGS "${COMMON_CXX_FLAGS}  /W4 /arch:AVX2 /fp:fast")
endif()




add_subdirectory(application)
#options
#option(BUILD_WIN "Whether or not to build the tests" ON)
#option(BUILD_JIT   "Whether or not to build the jit engine" ON)
#option(BUILD_REPL  "Whether or not to build the interpreter" ON)

#MESSAGE( STATUS "Building with the following options")
#MESSAGE( STATUS "BUILD JIT:                    " ${BUILD_JIT})
#MESSAGE( STATUS "BUILD REPL:                   " ${BUILD_REPL})
#MESSAGE( STATUS "BUILD TESTS:                  " ${BUILD_TESTS})
#adding core
#add_subdirectory(src/core)
#if(${BUILD_JIT} STREQUAL "ON")
#add_subdirectory(src/jit)
#endif()



add_subdirectory(mg_rayLib)
