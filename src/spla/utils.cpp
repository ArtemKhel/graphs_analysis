#include "triangles_counting.hpp"

namespace spla_utils
{

    spla::ref_ptr<spla::Matrix> load_graph(const std::string &path, bool triangular)
    {
        spla::MtxLoader loader;
        if (!loader.load(path, true, true, true))
        // if (!loader.load(path, false, true, true))
        {
            throw std::runtime_error("Failed to load graph: " + path);
        }

        const auto n = loader.get_n_rows();
        auto A = spla::Matrix::make(n, n, spla::INT);
        const auto &Ai = loader.get_Ai();
        const auto &Aj = loader.get_Aj();

        for (std::size_t k = 0; k < loader.get_n_values(); ++k)
        {
            if (!triangular || Ai[k] > Aj[k])
            {
                A->set_int(Ai[k], Aj[k], 0); // TODO: this
            }
        }

        return A;
    }

}