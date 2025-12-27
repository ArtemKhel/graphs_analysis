#include "triangles_counting.hpp"
#include "utils.hpp"
#include <stdexcept>
#include <fstream>
#include <sstream>
#include <chrono>
#include <iostream>
#include <algorithm>

namespace tc_graphblas
{
    uint64_t triangles_counting(GrB_Matrix A, bool triangular)
    {
        GrB_Matrix squared;
        GrB_Index n;
        GrB_Matrix_nrows(&n, A);

        GrB_Matrix_new(&squared, GrB_UINT64, n, n);

        GrB_mxm(squared, A, nullptr, GxB_PLUS_TIMES_UINT64, A, A, nullptr);

        uint64_t sum = 0;
        GrB_Matrix_reduce_UINT64(&sum, nullptr, GrB_PLUS_MONOID_UINT64, squared, nullptr);

        GrB_Matrix_free(&squared);

        if (!triangular && sum % 6 != 0)
        {
            std::cerr << "Warning: triangle count is not multiple of 6 in Burkhardt algorithm" << std::endl;
        }

        return triangular ? sum : sum / 6;
    }

    uint64_t burkhardt(GrB_Matrix A)
    {
        return triangles_counting(A, false);
    }

    uint64_t sandia(GrB_Matrix A)
    {
        return triangles_counting(A, true);
    }

    std::vector<double> benchmark(const char *filename, bool triangular, const int num_iters)
    {
        std::vector<double> iteration_times;
        iteration_times.reserve(num_iters);

        GrB_init(GrB_NONBLOCKING);

        GrB_Matrix A;
        A = graphblas_utils::load_graph(filename, triangular);

        for (int i = 0; i < num_iters; ++i)
        {
            auto start = std::chrono::high_resolution_clock::now();
            uint64_t answer;
            if (triangular)
            {
                answer = sandia(A);
            }
            else
            {
                answer = burkhardt(A);
            }
            auto end = std::chrono::high_resolution_clock::now();

            std::chrono::duration<double> elapsed = end - start;

            std::cout << (triangular ? "GB_Sandia" : "GB_Burkhardt")
                      << " Iteration " << i + 1 << ": " << elapsed.count() << " s" << std::endl;
            iteration_times.push_back(elapsed.count());
        }

        GrB_finalize();

        return iteration_times;
    }

}