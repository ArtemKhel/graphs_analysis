#pragma once
#include <spla.hpp>
#include <vector>
#include <string>

namespace tc_spla
{
    uint64_t sandia(spla::ref_ptr<spla::Matrix> A);
    uint64_t burkhardt(spla::ref_ptr<spla::Matrix> A);
    std::vector<double> benchmark(const char *filename, bool triangular, int num_runs);
}