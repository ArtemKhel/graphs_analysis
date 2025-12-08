#pragma once
#include <GraphBLAS.h>
#include <vector>
#include <string>
#include "utils.hpp"

namespace tc_gb
{
    int64_t triangles_counting(GrB_Matrix A, bool triangular);

    int64_t burkhardt(GrB_Matrix A);

    int64_t sandia(GrB_Matrix A);

    std::vector<double> benchmark(const char *filename, bool triangular, const int num_iters);
}