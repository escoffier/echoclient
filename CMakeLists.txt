cmake_minimum_required(VERSION 2.8.10)
project(echo_client C CXX)

option(BRPC_LINK_SO "Whether brpc are linked dynamically" OFF)

include(FindThreads)
#include(FindProtobuf)
include_directories(${CMAKE_CURRENT_BINARY_DIR}/src )

# find_path(GPERFTOOLS_INCLUDE_DIR NAMES gperftools/heap-profiler.h)
# find_library(GPERFTOOLS_LIBRARIES NAMES tcmalloc_and_profiler)
# include_directories(${GPERFTOOLS_INCLUDE_DIR})

# find_path(BRPC_INCLUDE_PATH NAMES brpc/server.h)
# if(BRPC_LINK_SO)
#     find_library(BRPC_LIB NAMES brpc)
# else()
#     find_library(BRPC_LIB NAMES libbrpc.a brpc)
# endif()
# if((NOT BRPC_INCLUDE_PATH) OR (NOT BRPC_LIB))
#     message(FATAL_ERROR "Fail to find brpc")
# endif()
# include_directories(${BRPC_INCLUDE_PATH})

# find_path(GFLAGS_INCLUDE_PATH gflags/gflags.h)
# find_library(GFLAGS_LIBRARY NAMES gflags libgflags)
# if((NOT GFLAGS_INCLUDE_PATH) OR (NOT GFLAGS_LIBRARY))
#     message(FATAL_ERROR "Fail to find gflags")
# endif()
# include_directories(${GFLAGS_INCLUDE_PATH})

# execute_process(
#     COMMAND bash -c "grep \"namespace [_A-Za-z0-9]\\+ {\" ${GFLAGS_INCLUDE_PATH}/gflags/gflags_declare.h | head -1 | awk '{print $2}' | tr -d '\n'"
#     OUTPUT_VARIABLE GFLAGS_NS
# )
# if(${GFLAGS_NS} STREQUAL "GFLAGS_NAMESPACE")
#     execute_process(
#         COMMAND bash -c "grep \"#define GFLAGS_NAMESPACE [_A-Za-z0-9]\\+\" ${GFLAGS_INCLUDE_PATH}/gflags/gflags_declare.h | head -1 | awk '{print $3}' | tr -d '\n'"
#         OUTPUT_VARIABLE GFLAGS_NS
#     )
# endif()


set(CMAKE_CPP_FLAGS "${DEFINE_CLOCK_GETTIME} -DGFLAGS_NS=${GFLAGS_NS}")
set(CMAKE_CXX_FLAGS "${CMAKE_CPP_FLAGS} -DNDEBUG -O2 -D__const__= -pipe -W -Wall -Wno-unused-parameter -fPIC -fno-omit-frame-pointer")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DBRPC_ENABLE_CPU_PROFILER")

if(CMAKE_VERSION VERSION_LESS "3.1.3")
    if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
    endif()
    if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
    endif()
else()
    set(CMAKE_CXX_STANDARD 11)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)
endif()


find_library(SSL_LIB NAMES ssl)
if (NOT SSL_LIB)
    message(FATAL_ERROR "Fail to find ssl")
endif()

find_library(CRYPTO_LIB NAMES crypto)
if (NOT CRYPTO_LIB)
    message(FATAL_ERROR "Fail to find crypto")
endif()

set(DYNAMIC_LIB
    ${CMAKE_THREAD_LIBS_INIT}
    ${GFLAGS_LIBRARY}
    ${PROTOBUF_LIBRARIES}
    ${LEVELDB_LIB}
    ${SSL_LIB}
    ${CRYPTO_LIB}
    dl
    event
    ${GPERFTOOLS_LIBRARIES}
    )


set(SOURCES
    ${CMAKE_SOURCE_DIR}/src/main.cpp)
    #${CMAKE_SOURCE_DIR}/src/eventthread.cpp)

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/output/bin)
include_directories(${CMAKE_SOURCE_DIR}/src)
include_directories(${CMAKE_SOURCE_DIR}/src/spdlog/include)

link_libraries(event pthread)
add_executable(${PROJECT_NAME} ${SOURCES} )