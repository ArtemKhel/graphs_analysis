#include <GraphBLAS.h>
#include <iostream>
#include <filesystem>
#include <fstream>
#include <vector>
#include <random>
#include <chrono>
#include "graphblas/msbfs.hpp"
#include "graphblas/utils.hpp"
#include "spla/utils.hpp"
#include "spla/msbfs.hpp"

int main(int argc, char* argv[]) {
    if (argc < 3) {
        std::cerr << "Usage: " << argv[0] << " <datasets_folder> <num_iters>" << std::endl;
        return 1;
    }
    std::string folder = argv[1];
    int num_iters = std::stoi(argv[2]);
    std::vector<int> n_start_list = {2, 8, 32, 128, 512};
    std::ofstream csv("msbfs_bench.csv");
    csv << "algo,dataset,n_start_vert,time" << std::endl;
    std::mt19937 rng(42);
    for (const auto& entry : std::filesystem::directory_iterator(folder)) {
        if (entry.is_regular_file() && entry.path().extension() == ".txt") {
            std::string dataset = entry.path().filename().string();
            std::string dataset_path = entry.path().string();
            std::cout << "\nRunning msbfs benchmarks for dataset: " << dataset << std::endl;
            GrB_init(GrB_NONBLOCKING);
            GrB_Matrix A = gb_utils::load_graph(dataset_path, false);
            GrB_Index n;
            GrB_Matrix_nrows(&n, A);
            std::vector<GrB_Index> all_vertices(n);
            for (GrB_Index i = 0; i < n; ++i) all_vertices[i] = i;
            for (int n_start : n_start_list) {
                if (n_start > n) continue;
                for (int iter = 0; iter < num_iters; ++iter) {
                    std::shuffle(all_vertices.begin(), all_vertices.end(), rng);
                    std::vector<GrB_Index> starts(all_vertices.begin(), all_vertices.begin() + n_start);

                    auto start = std::chrono::high_resolution_clock::now();
                    GrB_Matrix parent = msbfs(A, starts);
                    auto end = std::chrono::high_resolution_clock::now();
                    std::chrono::duration<double> elapsed = end - start;

                    csv << "GB_MSBFS," << dataset << "," << n_start << "," << elapsed.count() << std::endl;
                    GrB_Matrix_free(&parent);
                }
            }
            GrB_Matrix_free(&A);
            GrB_finalize();


            auto B = spla_utils::load_graph(dataset_path, false);
            for (int n_start : n_start_list) {
                if (n_start > B->get_n_rows()) continue;
                for (int iter = 0; iter < num_iters; ++iter) {
                    std::shuffle(all_vertices.begin(), all_vertices.end(), rng);
                    std::vector<int> starts(all_vertices.begin(), all_vertices.begin() + n_start);

                    auto start = std::chrono::high_resolution_clock::now();
                    spla::ref_ptr<spla::Matrix> parent = msbfs_spla::msbfs(B, starts);
                    auto end = std::chrono::high_resolution_clock::now();
                    std::chrono::duration<double> elapsed = end - start;

                    csv << "SPLA_MSBFS," << dataset << "," << n_start << "," << elapsed.count() << std::endl;
                }
            }
        }
    }
    csv.close();
    return 0;
}
