#include "triangles_counting.hpp"


namespace tc_spla
{

    int triangles_counting(spla::ref_ptr<spla::Matrix> A, bool triangular)
    {
        using namespace spla;

        const uint n = A->get_n_cols();

        ref_ptr<Matrix> B = Matrix::make(n, n, INT);

        ref_ptr<Scalar> zero = Scalar::make_int(0);
        ref_ptr<Scalar> result = Scalar::make(INT);

        exec_mxmT_masked(B, // result
                         A, // mask
                         A, // left
                         A, // right
                         MULT_INT, PLUS_INT, GTZERO_INT, zero);
        exec_m_reduce(result, zero, B, PLUS_INT);

        int count = result->as_int();

        B->clear();

        return triangular ? count : count / 6;
    }

    int sandia(spla::ref_ptr<spla::Matrix> A)
    {
        return triangles_counting(A, true);
    }

    int burkhardt(spla::ref_ptr<spla::Matrix> A)
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
            Library::get()->set_force_no_acceleration(true);

            ref_ptr<Matrix> A = spla_utils::load_graph(filename, triangular);

            for (int i = 0; i < num_iters; ++i)
            {
                auto start = std::chrono::high_resolution_clock::now();
                int answer;
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
                          << " Iteration " << i + 1 << ": " << elapsed.count() << " s"
                          << ", triangles: " << answer << "\n";
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