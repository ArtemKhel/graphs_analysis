#include "msbfs.hpp"
#include <stdexcept>

GrB_Matrix msbfs(GrB_Matrix A, const std::vector<GrB_Index> &sources)
{
    GrB_Index n;
    GrB_Matrix_nrows(&n, A);
    GrB_Index nsrc = sources.size();

    GrB_Matrix front, visited, parent, next_parents;
    GrB_Matrix_new(&front, GrB_BOOL, nsrc, n);
    GrB_Matrix_new(&visited, GrB_BOOL, nsrc, n);
    GrB_Matrix_new(&parent, GrB_INT64, nsrc, n);
    GrB_Matrix_new(&next_parents, GrB_INT64, nsrc, n);

    for (GrB_Index i = 0; i < nsrc; ++i)
    {
        GrB_Matrix_setElement_BOOL(front, true, i, sources[i]);
        GrB_Matrix_setElement_BOOL(visited, true, i, sources[i]);
        GrB_Matrix_setElement_INT64(parent, sources[i], i, sources[i]);
    }

    while (true)
    {
        // next_parents = front * A
        GrB_mxm(next_parents, GrB_NULL, GrB_NULL, GxB_ANY_SECONDI_INT64, front, A, GrB_NULL);

        // not_visited = !visited
        GrB_Matrix not_visited;
        GrB_Matrix_new(&not_visited, GrB_BOOL, nsrc, n);
        GrB_Matrix_assign_BOOL(not_visited, visited, GrB_NULL, true, GrB_ALL, nsrc, GrB_ALL, n, GrB_DESC_C);

        // mask next_parents with not_visited
        GrB_Matrix masked_next;
        GrB_Matrix_new(&masked_next, GrB_INT64, nsrc, n);
        GrB_Matrix_apply(masked_next, not_visited, GrB_NULL, GrB_IDENTITY_INT64, next_parents, GrB_NULL);

        // Check for done
        GrB_Index nvals;
        GrB_Matrix_nvals(&nvals, masked_next);
        if (nvals == 0)
        {
            GrB_Matrix_free(&not_visited);
            GrB_Matrix_free(&masked_next);
            break;
        }

        // Update visited and parent
        GrB_Matrix_assign(parent, masked_next, GrB_NULL, masked_next, GrB_ALL, nsrc, GrB_ALL, n, GrB_DESC_S);

        // Prepare next front
        GrB_Matrix_free(&front);
        GrB_Matrix_new(&front, GrB_BOOL, nsrc, n);
        GrB_Matrix_assign_BOOL(front, masked_next, GrB_NULL, true, GrB_ALL, nsrc, GrB_ALL, n, GrB_DESC_S);
        GrB_Matrix_eWiseAdd_BinaryOp(visited, GrB_NULL, GrB_NULL, GrB_LOR, visited, front, GrB_NULL);

        GrB_Matrix_free(&not_visited);
        GrB_Matrix_free(&masked_next);
    }
    GrB_Matrix_free(&next_parents);
    GrB_Matrix_free(&front);
    GrB_Matrix_free(&visited);
    return parent;
}
