#include "msbfs.hpp"
#include <spla.hpp>
#include <vector>
#include <functional>
#include <algorithm>
#include <string>
#include <stdexcept>
#include <limits.h>
#include "utils.hpp"

using namespace spla;

void reduce_by_vectors(ref_ptr<Scalar> &r, ref_ptr<Matrix> &m, ref_ptr<OpBinary> &op)
{
    ref_ptr<Vector> v = Vector::make(m->get_n_cols(), INT);

    exec_m_reduce_by_row(v, m, op, Scalar::make_int(0));
    exec_v_reduce(r, Scalar::make_int(0), v, op);
}

void apply_mask_on_front(ref_ptr<Matrix> &front, ref_ptr<Matrix> &mask)
{
    ref_ptr<Scalar> mask_nnz = Scalar::make_int(0);
    reduce_by_vectors(mask_nnz, mask, PLUS_INT);
    std::vector<T_INT> bf1, bf2, bf3;

    ref_ptr<MemView> rows_view = MemView::make(bf1.data(), mask_nnz->as_int());
    ref_ptr<MemView> cols_view = MemView::make(bf2.data(), mask_nnz->as_int());
    ref_ptr<MemView> values_view = MemView::make(bf3.data(), mask_nnz->as_int());

    mask->read(rows_view, cols_view, values_view);

    T_INT *rows = static_cast<T_INT *>(rows_view->get_buffer());
    T_INT *cols = static_cast<T_INT *>(cols_view->get_buffer());
    T_INT *values = static_cast<T_INT *>(values_view->get_buffer());

    for (auto i = 0; i < mask_nnz->as_int(); ++i)
    {
        if (values[i] != INT_MAX)
        {
            front->set_int(rows[i], cols[i], 0);
        }
    }
}
namespace msbfs_spla
{
    ref_ptr<Matrix> msbfs(ref_ptr<Matrix> A, const std::vector<int> &sources, bool accelerated)
    {
        Library::get()->set_force_no_acceleration(!accelerated);

        auto nsrc = sources.size();
        auto nrows = A->get_n_rows();

        ref_ptr<Matrix> prev_front = Matrix::make(nsrc, nrows, INT);
        prev_front->set_fill_value(Scalar::make_int(0));

        ref_ptr<Scalar> front_size = Scalar::make_int(nsrc);

        bool fronts_empty = false;

        ref_ptr<Matrix> parents = Matrix::make(nsrc, nrows, INT);
        parents->set_fill_value(Scalar::make_int(0));

        ref_ptr<Matrix> front = Matrix::make(nsrc, nrows, INT);
        front->set_fill_value(Scalar::make_int(0));

        ref_ptr<Matrix> new_parents = Matrix::make(nsrc, nrows, INT);
        new_parents->set_fill_value(Scalar::make_int(0));

        ref_ptr<Matrix> mask = Matrix::make(nsrc, nrows, INT);
        mask->set_fill_value(Scalar::make_int(0));

        int i = 0;
        for (auto s : sources)
        {
            prev_front->set_int(i, s, s + 1);
            parents->set_int(i, s, s + 1);
            ++i;
        }

        auto MIN_NON_ZERO_INT = OpBinary::make_int(
            "min_non_zero_int",
            "min_non_zero_int"
            "(int a, int b) {"
            "    if (a == 0) return b;"
            "    return (a < b) ? a : b;"
            "}",
            [](T_INT a, T_INT b)
            {
                if (a == 0)
                {
                    return b;
                }
                return std::min(a, b);
            });

        while (!fronts_empty)
        {
            exec_mxm(front, prev_front, A, FIRST_INT, MIN_NON_ZERO_INT, Scalar::make_int(0));

            exec_m_emult(mask, front, parents, BONE_INT);

            exec_m_eadd(new_parents, parents, front, FIRST_INT);

            apply_mask_on_front(front, mask);

            reduce_by_vectors(front_size, front, PLUS_INT);

            std::swap(front, prev_front);
            std::swap(new_parents, parents);

            fronts_empty = front_size->as_int() == 0;
        }

        return parents;
    }
}