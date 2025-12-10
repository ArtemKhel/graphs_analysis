#include "triangles_counting.hpp"
#include "utils.hpp"
#include <iostream>

namespace tc_spla
{

    uint64_t triangles_counting(spla::ref_ptr<spla::Matrix> A, bool triangular)
    {
        using namespace spla;

        const uint n = A->get_n_cols();

        ref_ptr<Matrix> B = Matrix::make(n, n, UINT);

        ref_ptr<Scalar> zero = Scalar::make_uint(0);
        ref_ptr<Scalar> result = Scalar::make(UINT);

        exec_mxmT_masked(B, // result
                         A, // mask
                         A, // left
                         A, // right
                         MULT_UINT, PLUS_UINT, GTZERO_UINT, zero);
        exec_m_reduce(result, zero, B, PLUS_UINT);

        uint64_t count = result->as_uint();

        B->clear();

        return triangular ? count : count / 6;
    }

    uint64_t sandia(spla::ref_ptr<spla::Matrix> A)
    {
        return triangles_counting(A, true);
    }

    uint64_t burkhardt(spla::ref_ptr<spla::Matrix> A)
    {
        return triangles_counting(A, false);
    }

    std::vector<double> benchmark(const char *filename, bool triangular, const int num_iters)
    {
        using namespace spla;

        std::vector<double> iteration_times;
        iteration_times.reserve(num_iters);

        try
        {
            ref_ptr<Matrix> A = spla_utils::load_graph(filename, triangular);

            Library::get()->set_force_no_acceleration(true);

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

                std::cout << (triangular ? "SPLA_Sandia" : "SPLA_Burkhardt")
                          << " Iteration " << i + 1 << ": " << elapsed.count() << " s" << std::endl;
                iteration_times.push_back(elapsed.count());
            }
        }
        catch (const std::exception &e)
        {
            std::cerr << "Error: " << e.what() << std::endl;
            throw;
        }

        return iteration_times;
    }
}