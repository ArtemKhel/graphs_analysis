#pragma once
#include <spla.hpp>
#include <vector>
#include <string>

namespace msbfs_spla
{
    spla::ref_ptr<spla::Matrix> msbfs(spla::ref_ptr<spla::Matrix> A, const std::vector<int> &sources, bool accelerated);
}
