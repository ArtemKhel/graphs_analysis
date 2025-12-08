#pragma once
#include <GraphBLAS.h>
#include <vector>


GrB_Matrix msbfs(GrB_Matrix A, const std::vector<GrB_Index>& sources);
