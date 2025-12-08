import sys
import os
import time
import csv
import random
import numpy as np
from graphblas import Matrix, Vector, semiring, binary


class MultiSourceBFS:
    def __init__(self):
        self.graph = None

    def load_from_file(self, path, undirected=True):
        edges = np.loadtxt(path, dtype=int, delimiter=None)
        if edges.ndim == 1:
            edges = edges.reshape(1, -1)
        u = edges[:, 0].tolist()
        v = edges[:, 1].tolist()
        if undirected:
            row_indices = u + v
            col_indices = v + u
        else:
            row_indices = u
            col_indices = v
        self.graph = Matrix.from_coo(row_indices, col_indices, True)

    def run(self, sources):
        if self.graph is None:
            raise RuntimeError("Graph not loaded. Call load_from_file first.")

        n = self.graph.nrows
        sources_number = len(sources)

        front = Matrix(bool, sources_number, n)
        visited = Matrix(bool, sources_number, n)
        parent = Matrix(int, sources_number, n)

        for i, source in enumerate(sources):
            front[i, source] = True
            visited[i, source] = True
            parent[i, source] = source

        while front.nvals > 0:
            next_parents = front.mxm(self.graph, semiring.any_secondi)

            not_visited = Matrix(bool, sources_number, n)
            not_visited[:, :] = 1
            not_visited = not_visited.ewise_add(visited, op=binary.minus)

            next_parents = next_parents.select(not_visited)

            if next_parents.nvals == 0:
                break

            visited(mask=next_parents.S) << True
            parent(mask=next_parents.S) << next_parents

            front = Matrix(bool, sources_number, n)
            front(mask=next_parents.S) << True

        return parent


def main():
    if len(sys.argv) < 3:
        print("Usage: ms_bfs.py <edge_file> <start1>[,<start2>,...]")
        sys.exit(1)

    path = sys.argv[1]
    sources = list(map(int, sys.argv[2].split(",")))

    msbfs = MultiSourceBFS()
    msbfs.load_from_file(path, undirected=True)
    parent = msbfs.run(sources)
    dense = parent.to_dense(fill_value=-1)
    print(dense)


def bench(data_dir, num_iters, out_csv='gb_msbfs_bench.csv'):
    random.seed(42)
    n_start_list = [2**(2*i) for i in range(5)] # 1,4,16,64,256
    datasets = [f for f in os.listdir(data_dir) if f.endswith('.txt')]

    with open(out_csv, 'w', newline='') as csvfile:
        writer = csv.writer(csvfile)
        writer.writerow(['GB_MSBFS', 'dataset', 'n_start_vert', 'time'])
        for dataset in datasets:
            path = os.path.join(data_dir, dataset)
            msbfs = MultiSourceBFS()
            msbfs.load_from_file(path, undirected=True)
            n = msbfs.graph.nrows
            for n_start in n_start_list:
                for _ in range(num_iters):
                    if n_start > n:
                        continue
                    starts = random.sample(range(n), n_start)
                    t0 = time.time()
                    msbfs.run(starts)
                    t1 = time.time()
                    writer.writerow(['GB_MSBFS', dataset, n_start, t1-t0])


if __name__ == '__main__':
    # main()
    data_dir = sys.argv[1]
    num_iters = int(sys.argv[2])
    bench(data_dir, num_iters)
