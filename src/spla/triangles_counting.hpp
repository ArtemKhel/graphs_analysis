#pragma once
#include <spla.hpp>
#include <vector>
#include <string>
#include "utils.hpp"

namespace tc_spla {

    int sandia(spla::ref_ptr<spla::Matrix> A);

    int burkhardt(spla::ref_ptr<spla::Matrix> A);

    std::vector<double> benchmark(const char* filename, bool triangular, int num_runs);

}