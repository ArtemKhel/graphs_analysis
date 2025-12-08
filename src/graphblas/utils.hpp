#pragma once
#include <GraphBLAS.h>
#include <string>
#include <vector>

namespace gb_utils {
    GrB_Info extract_upper(GrB_Matrix *U, GrB_Matrix A, bool strict);
    GrB_Matrix load_graph(const std::string &filepath, bool triangular);
}
