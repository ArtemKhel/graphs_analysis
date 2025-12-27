cmake_minimum_required(VERSION 3.10)
project(triangles_counting)

set(CMAKE_CXX_STANDARD 20)

# set(CMAKE_BUILD_TYPE Debug CACHE STRING "" FORCE)

add_compile_options(-fno-omit-frame-pointer -g)

set(SPLA_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(SPLA_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(SPLA_BUILD_OPENCL ON CACHE BOOL "" FORCE)

add_subdirectory(spla)
add_subdirectory(GraphBLAS)

add_executable(bench_tc
    src/bench_tc.cpp

    src/graphblas/triangles_counting.cpp
    src/graphblas/utils.cpp

    src/spla/triangles_counting.cpp
    src/spla/utils.cpp
)

target_link_libraries(bench_tc PRIVATE
        spla
        GraphBLAS
)

target_include_directories(bench_tc PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/spla/include
        ${CMAKE_CURRENT_SOURCE_DIR}/spla/examples
        ${CMAKE_CURRENT_SOURCE_DIR}/GraphBLAS/Include
        ${CMAKE_CURRENT_SOURCE_DIR}/GraphBLAS/Source
)

add_executable(bench_msbfs
    src/bench_msbfs.cpp

    src/graphblas/msbfs.cpp
    src/graphblas/utils.cpp

    src/spla/msbfs.cpp
    src/spla/utils.cpp
)

target_link_libraries(bench_msbfs PRIVATE 
    GraphBLAS
    spla
)

target_include_directories(bench_msbfs PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/spla/include
    ${CMAKE_CURRENT_SOURCE_DIR}/spla/examples
    ${CMAKE_CURRENT_SOURCE_DIR}/GraphBLAS/Include
    ${CMAKE_CURRENT_SOURCE_DIR}/GraphBLAS/Source
)
