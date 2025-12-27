#include "triangles_counting.hpp"
#include "utils.hpp"
#include <iostream>

namespace tc_spla
{
    using namespace spla;

    void triangles_counting(
        int &ntrins,
        const ref_ptr<Matrix> &A,
        const ref_ptr<Matrix> &B)
    {
        ref_ptr<Scalar> zero = Scalar::make_int(0);
        ref_ptr<Scalar> result = Scalar::make(INT);
        auto stat = exec_mxmT_masked(B, // result
                                     A, // mask
                                     A, // left
                                     A, // right
                                     MULT_INT, PLUS_INT, GTZERO_INT, zero);
        stat = exec_m_reduce(result, zero, B, PLUS_INT);
        ntrins = result->as_int();
    }

    void sandia(int &ntrins, const ref_ptr<Matrix> &A, const ref_ptr<Matrix> &B)
    {
        triangles_counting(ntrins, A, B);
    }

    void burkhardt(int &ntrins, const ref_ptr<Matrix> &A, const ref_ptr<Matrix> &B)
    {
        triangles_counting(ntrins, A, B);
        if (ntrins % 6 != 0)
        {
            std::cerr << "Warning: triangle count is not multiple of 6 in Burkhardt algorithm" << std::endl;
        }
        ntrins = ntrins / 6;
    }

    std::vector<double> benchmark(const char *filename, bool triangular, const int num_iters, bool accelerated)
    {
        using namespace spla;

        std::vector<double> iteration_times;
        iteration_times.reserve(num_iters);

        ref_ptr<Matrix> A = spla_utils::load_graph(filename, triangular);
        uint N = A->get_n_rows();
        ref_ptr<Matrix> B_cpu = Matrix::make(N, N, INT);
        ref_ptr<Matrix> B_acc = Matrix::make(N, N, INT);

        Library::get()->set_force_no_acceleration(!accelerated);

        for (int i = 0; i < num_iters; ++i)
        {
            auto start = std::chrono::high_resolution_clock::now();
            int answer = 0;
            if (triangular)
            {
                sandia(answer, A, B_acc);
            }
            else
            {
                burkhardt(answer, A, B_acc);
            }
            auto end = std::chrono::high_resolution_clock::now();

            if (accelerated)
            {
                B_acc->clear();
            }
            else
            {
                B_cpu->clear();
            }

            std::chrono::duration<double> elapsed = end - start;

            std::cout << (triangular ? (accelerated ? "SPLAGPU_Sandia" : "SPLA_Sandia") : (accelerated ? "SPLAGPU_Burkhardt" : "SPLA_Burkhardt"))
                      << " Iteration " << i + 1 << ": " << elapsed.count() << " s" << std::endl;
            iteration_times.push_back(elapsed.count());
        }

        return iteration_times;
    }
}