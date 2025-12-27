# Анализ Графов

Реализация и анализ алгоритмов Multiple Source Parent BFS и Triange counting (Burkhardt, Sandia) на графах с использованием библиотек GraphBLAS и SPLA

[Презентация](./presentation.pdf)

## Requirements

- CMake (version 3.20+)
- C++20 

## Getting Started

```sh
git clone --recurse-submodules --shallow-submodules --depth 1 https://github.com/artemkhel/graphs_analysis
cd graph-analysis

cmake -S . -B build 
cmake --build build -- -j$(nproc)
```

```sh
build/bench_msbfs <path/to/dataset/dir> <n_iters>
build/bench_tc <path/to/dataset/dir> <n_iters>
```
