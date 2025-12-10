#include "msbfs.hpp"
#include <spla.hpp>
#include <vector>
#include <functional>
#include <algorithm>
#include <string>
#include <stdexcept>
#include <limits.h>
#include "utils.hpp"

void ReduceByVectors(spla::ref_ptr<spla::Scalar> &r, spla::ref_ptr<spla::Matrix> &m,
                     spla::ref_ptr<spla::OpBinary> &op)
{
    spla::ref_ptr<spla::Vector> v = spla::Vector::make(m->get_n_cols(), spla::INT);

    spla::exec_m_reduce_by_row(v, m, op, spla::Scalar::make_int(0));
    spla::exec_v_reduce(r, spla::Scalar::make_int(0), v, op);
}

void ApplyMaskOnFront(spla::ref_ptr<spla::Matrix> &front,
                      spla::ref_ptr<spla::Matrix> &mask)
{
    spla::ref_ptr<spla::Scalar> nnz_mask_count = spla::Scalar::make_int(0);
    ReduceByVectors(nnz_mask_count, mask, spla::PLUS_INT);
    std::vector<spla::T_INT> bf1, bf2, bf3;

    spla::ref_ptr<spla::MemView> keys1_view =
        spla::MemView::make(bf1.data(), nnz_mask_count->as_int());
    spla::ref_ptr<spla::MemView> keys2_view =
        spla::MemView::make(bf2.data(), nnz_mask_count->as_int());
    spla::ref_ptr<spla::MemView> values_view =
        spla::MemView::make(bf3.data(), nnz_mask_count->as_int());

    mask->read(keys1_view, keys2_view, values_view);

    spla::T_INT *keys1 = static_cast<spla::T_INT *>(keys1_view->get_buffer());
    spla::T_INT *keys2 = static_cast<spla::T_INT *>(keys2_view->get_buffer());
    spla::T_INT *values = static_cast<spla::T_INT *>(values_view->get_buffer());

    for (spla::T_INT i = 0; i < nnz_mask_count->as_int(); ++i)
    {
        if (values[i] != INT_MAX)
        {
            front->set_int(keys1[i], keys2[i], 0);
        }
    }
}
namespace msbfs_spla
{
    spla::ref_ptr<spla::Matrix> msbfs(spla::ref_ptr<spla::Matrix> A, const std::vector<int> &sources)
    {
        auto nsrc = sources.size();
        auto nrows = A->get_n_rows();

        spla::ref_ptr<spla::Matrix> prev_fronts =
            spla::Matrix::make(nsrc, nrows, spla::INT);
        prev_fronts->set_fill_value(spla::Scalar::make_int(0));

        spla::ref_ptr<spla::Scalar> frontier_size = spla::Scalar::make_int(nsrc);

        bool fronts_empty = false;

        auto parents = spla::Matrix::make(nsrc, nrows, spla::INT);
        parents->set_fill_value(spla::Scalar::make_int(0));

        spla::ref_ptr<spla::Matrix> p = spla::Matrix::make(nsrc, nrows, spla::INT);
        p->set_fill_value(spla::Scalar::make_int(0));

        spla::ref_ptr<spla::Matrix> p_updated =
            spla::Matrix::make(nsrc, nrows, spla::INT);
        p_updated->set_fill_value(spla::Scalar::make_int(0));

        spla::ref_ptr<spla::Matrix> p_mask =
            spla::Matrix::make(nsrc, nrows, spla::INT);
        p_mask->set_fill_value(spla::Scalar::make_int(0));

        int i = 0;
        for (auto s : sources)
        {
            prev_fronts->set_int(i, s, s + 1);
            parents->set_int(i, s, s + 1);
            ++i;
        }

        auto min_non_zero_int =
            spla::OpBinary::make_int(
                std::string("MIN_NON_ZERO_INT"),
                std::string("test"),
                std::function<spla::T_INT(spla::T_INT, spla::T_INT)>(
                    [](spla::T_INT a, spla::T_INT b)
                    {
                        if (a == 0)
                        {
                            return b;
                        }
                        return std::min(a, b);
                    }));

        while (!fronts_empty)
        {
            spla::exec_mxm(p, prev_fronts, A, spla::FIRST_INT, min_non_zero_int, spla::Scalar::make_int(0));

            spla::exec_m_emult(p_mask, p, parents, spla::BONE_INT);

            spla::exec_m_eadd(p_updated, parents, p, spla::FIRST_INT);

            ApplyMaskOnFront(p, p_mask);

            ReduceByVectors(frontier_size, p, spla::PLUS_INT);

            std::swap(p, prev_fronts);
            std::swap(p_updated, parents);

            fronts_empty = frontier_size->as_int() == 0;
        }

        return parents;
    }
}