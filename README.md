# Minimum Edge Cut of a Connected Graph

## Problem Description

Given a simple, connected, undirected, edge-weighted graph **G(V, E)** with **n** nodes,
the goal is to find a minimum-weight edge cut between two disjoint subsets of nodes.

This problem was first solved sequentially (see the `seq` folder).   
Then, it was parallelized using **task parallelism** with OpenMP (`task` folder), followed by a version based on **data parallelism** using OpenMP as well (`data` folder).   
Finally, a **distributed** version was implemented using the **Master-Slave algorithm** with MPI (`mpi` folder), tested on a cluster with a large number of nodes. In this distributed version, each slave process also used **task parallelism** locally via OpenMP.

## Input

- `n` — number of nodes in the graph G
    - Natural number: `10 ≤ n < 100`
- `G(V, E)` — simple, connected, undirected, edge-weighted graph
    - Edge weights are in the interval `[71, 149]`
- `a` — size of the first subset
    - Natural number: `5 ≤ a ≤ n / 2`

## Task

Partition the set of nodes **V** into two disjoint subsets **X** and **Y** such that:

- `|X| = a` and `|Y| = n - a` *(or vice versa)*
- The sum of the weights of all edges `{u, v}` where `u ∈ X` and `v ∈ Y` is **minimized**

This sum represents the **weight of the edge cut** between `X` and `Y`.

## Output

The algorithm should return:

- The node subsets **X** and **Y**
- The **total weight** of the cut

## Example

### Sequential Version

```bash
cd seq/
make
./seq 15 graf_mhr/graf_30_20.txt
```

Output:
```yaml
Min Cut weight: 13159
X: 0 1 2 3 9 10 12 14 16 17 19 21 27 28 29
Y: 4 5 6 7 8 11 13 15 18 20 22 23 24 25 26
Recursion: 34100503
Time: 15.6928 seconds
```

### Task Parallel Version

```bash
cd task/
make
./task 15 graf_mhr/graf_30_20.txt
```

Output:
```yaml
Min Cut weight: 13159
X: 0 1 2 3 9 10 12 14 16 17 19 21 27 28 29
Y: 4 5 6 7 8 11 13 15 18 20 22 23 24 25 26
Recursion: 34993230
Time: 5.58415 seconds
```

### Data Parallel Version

```bash
cd data/
make
./data 15 graf_mhr/graf_30_20.txt
```

Output:
```yaml
Min Cut weight: 13159
X: 0 1 2 3 9 10 12 14 16 17 19 21 27 28 29
Y: 4 5 6 7 8 11 13 15 18 20 22 23 24 25 26
Time: 5.35847 seconds
```

### MPI Version

The program was run on a cluster with 4 nodes and 48 threads, through the script `mpi_script.sh`.
You can also try running it locally on your computer, but the number of nodes should not exceed 
the number of available cores (for example, `mpirun -np 4 ./mpi 15 graf_mhr/graf_30_20.txt`).

Output on the cluster:
```yaml
Min Cut weight: 13159
X: 0 1 2 3 9 10 12 14 16 17 19 21 27 28 29
Y: 4 5 6 7 8 11 13 15 18 20 22 23 24 25 26

Time: 2.99861 seconds
```