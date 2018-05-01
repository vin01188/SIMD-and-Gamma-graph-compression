15853 Project
======================

Usage
--------

To run the graph algorithms using Streamvbyte or Gamma encoding, make sure one of the following to the top apps/Makefile 

```
GAMMA = 1 or
BYTESIMD = 1
```

Next, navigate to the apps folder and run Make. To run the code on the wiki graph, you can just run:

```
./encoder -s ../inputs/wiki-Talk_1 ../inputs/wiki-Talk-SIMD
./BFS -s -c ../inputs/wiki-Talk-SIMD
```

To run other graph algorithms, modify the Makefile to build all the graph algorithms
by editing the following line with additional algorithms:

```
ALL = encoder decoder BFS simpleBenchmark
```

If you add all the algos to the Makefile, you can also run the following
the benchmark multiple algorithms at once. Currently, this only works for 
the SIMD encoder. Change the input file in run_benchmarks.sh to the input file
you want (e.g ../inputs/wiki-Talk-SIMD) for the WikiTalk file.

```
chmod +x run_benchmarks.sh
./run_benchmarks.sh
```

To run the array benchmarks, run the following in the apps directory:

```
./simpleBenchmark
```
