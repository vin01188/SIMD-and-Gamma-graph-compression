#!/bin/bash

./BFS -s -c -rounds 200 ../inputs/wiki-Talk-SIMD
./BellmanFord -s -c -rounds 200 ../inputs/wiki-Talk-SIMD
./PageRank -s -c -rounds 1 ../inputs/wiki-Talk-SIMD
./Components -s -c -rounds 20 ../inputs/wiki-Talk-SIMD
./Radii -s -c -rounds 200 ../inputs/wiki-Talk-SIMD
./BC -s -c -rounds 200 ../inputs/wiki-Talk-SIMD
./BFS-bitvector -s -c -rounds 200 ../inputs/wiki-Talk-SIMD
./MIS -s -c -rounds 20 ../inputs/wiki-Talk-SIMD
# ./Triangle -s -c -rounds 100 ../inputs/wiki-Talk-SIMD
# ./CF -s -c -rounds 100 ../inputs/wiki-Talk-SIMD

