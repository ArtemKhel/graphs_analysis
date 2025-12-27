#include <GraphBLAS.h>
#include <iostream>
#include <filesystem>
#include <fstream>
#include <vector>
#include <random>
#include <chrono>
#include <algorithm>
#include "graphblas/msbfs.hpp"
#include "graphblas/utils.hpp"
#include "spla/utils.hpp"
#include "spla/msbfs.hpp"

int main(int argc, char *argv[])
{
    if (argc < 3)
    {
        std::cerr << "Usage: " << argv[0] << " <datasets_folder> <num_iters>" << std::endl;
        return 1;
    }
    const int SEED = 42;
    std::mt19937 rng(SEED);

    std::string folder = argv[1];
    int num_iters = std::stoi(argv[2]);

    std::vector<int> n_start_list = {4, 8, 16, 32, 64};

    std::ofstream csv("msbfs_bench.csv");
    csv << "algo,dataset,n_start_vert,time" << std::endl;
    for (const auto &entry : std::filesystem::directory_iterator(folder))
    {
        if (entry.is_regular_file() && entry.path().extension() == ".txt")
        {
            std::string dataset = entry.path().filename().string();
            std::string dataset_path = entry.path().string();
            std::cout << "\nRunning msbfs benchmarks for dataset: " << dataset << std::endl;

            GrB_init(GrB_NONBLOCKING);
            GrB_Matrix A = graphblas_utils::load_graph(dataset_path, false);
            GrB_Index n;
            GrB_Matrix_nrows(&n, A);
            std::vector<GrB_Index> all_vertices(n);
            for (GrB_Index i = 0; i < n; ++i)
            {
                all_vertices[i] = i;
            }
            for (int n_start : n_start_list)
            {
                if (n_start > n)
                    continue;
                std::cout << "N start = " << n_start << std::endl;
                for (int iter = 0; iter < num_iters; ++iter)
                {
                    std::cout << "." << std::flush;
                    std::shuffle(all_vertices.begin(), all_vertices.end(), rng);
                    std::vector<GrB_Index> starts(all_vertices.begin(), all_vertices.begin() + n_start);

                    auto start = std::chrono::high_resolution_clock::now();
                    GrB_Matrix parent = msbfs(A, starts);
                    auto end = std::chrono::high_resolution_clock::now();
                    std::chrono::duration<double> elapsed = end - start;

                    csv << "GB_MSBFS," << dataset << "," << n_start << "," << elapsed.count() << std::endl;
                    GrB_Matrix_free(&parent);
                }
                std::cout << std::endl;
            }
            GrB_Matrix_free(&A);
            GrB_finalize();
        }
    }

    rng.seed(SEED);
    for (const auto &entry : std::filesystem::directory_iterator(folder))
    {
        if (entry.is_regular_file() && entry.path().extension() == ".txt")
        {
            std::string dataset = entry.path().filename().string();
            std::string dataset_path = entry.path().string();

            auto B = spla_utils::load_graph(dataset_path, false);
            auto n = B->get_n_rows();
            std::vector<int> all_vertices(n);
            for (int i = 0; i < n; ++i)
            {
                all_vertices[i] = i;
            }
            for (int n_start : n_start_list)
            {
                if (n_start > B->get_n_rows())
                    continue;
                std::cout << "N start = " << n_start << std::endl;
                for (int iter = 0; iter < num_iters; ++iter)
                {
                    std::cout << "." << std::flush;
                    std::shuffle(all_vertices.begin(), all_vertices.end(), rng);
                    std::vector<int> starts(all_vertices.begin(), all_vertices.begin() + n_start);

                    auto start = std::chrono::high_resolution_clock::now();
                    msbfs_spla::msbfs(B, starts, false);
                    auto end = std::chrono::high_resolution_clock::now();
                    std::chrono::duration<double> elapsed = end - start;

                    auto start_gpu = std::chrono::high_resolution_clock::now();
                    auto parents = msbfs_spla::msbfs(B, starts, true);
                    auto end_gpu = std::chrono::high_resolution_clock::now();
                    std::chrono::duration<double> elapsed_gpu = end_gpu - start_gpu;
                    // spla_utils::print_matrix(parents);

                    csv << "SPLA_MSBFS," << dataset << "," << n_start << "," << elapsed.count() << std::endl;
                    csv << "SPLAGPU_MSBFS," << dataset << "," << n_start << "," << elapsed_gpu.count() << std::endl;
                }
                std::cout << std::endl;
            }
        }
    }
    csv.close();
    return 0;
}
