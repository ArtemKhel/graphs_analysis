#pragma once
#include <GraphBLAS.h>
#include <vector>
#include <string>

namespace tc_graphblas
{
    uint64_t triangles_counting(GrB_Matrix A, bool triangular);

    uint64_t burkhardt(GrB_Matrix A);

    uint64_t sandia(GrB_Matrix A);

    std::vector<double> benchmark(const char *filename, bool triangular, const int num_iters);
}