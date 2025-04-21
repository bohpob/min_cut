#include "CSolver.h"
#include <algorithm>
#include <iostream>
#include <limits>
#include <omp.h>

long long n_rec = 0; // Number of recursions

CSolver::CSolver(const int n, const int a, const vector<vector<int> > &graph)
    : n(n), a(a), graph(graph), min_cut_weight(numeric_limits<int>::max()) {
}

void CSolver::dfs(const int node, const int x_count, const int cut_weight, vector<int> &partition) {
    #pragma omp atomic update
    n_rec++; // Atomically increment the recursion counter

    // Prune the search if the current cut weight exceeds the best found so far
    if (cut_weight > min_cut_weight)
        return;

    // If all nodes are processed, check if a valid partition is found
    if (node == n) {
        // Ensure the subset X has exactly 'a' elements
        if (x_count == a) {
            #pragma omp critical
            {
                if (cut_weight == min_cut_weight) {
                    best_partitions.push_back(partition); // Store another optimal partition
                } else if (cut_weight < min_cut_weight) {
                    min_cut_weight = cut_weight; // Update the best cut weight found
                    best_partitions.clear(); // Clear previous partitions
                    best_partitions.push_back(partition); // Store the new best partition
                }
            }
        }
        return;
    }

    int delta_x = 0, delta_y = 0; // Contribution to cut weight when assigning node to X or Y
    for (int i = 0; i < node; i++) {
        if (partition[i] == 0)
            delta_x += graph[i][node]; // Adding edges between node and subset X
        else
            delta_y += graph[i][node]; // Adding edges between node and subset Y
    }

    int low_bound_x = 0, low_bound_y = 0, low_bound = 0; // Compute a lower bound for future cut weight
    for (int i = node; i < n; i++) {
        for (int j = 0; j < node; j++) {
            if (partition[j] == 0)
                low_bound_x += graph[j][i];
            else
                low_bound_y += graph[j][i];
        }
        low_bound += min(low_bound_x, low_bound_y); // Minimum possible contribution to the cut
        low_bound_x = 0, low_bound_y = 0;
    }

    // Stop if the minimum possible future cut weight is already worse than the best found
    if (low_bound + cut_weight > min_cut_weight) {
        return;
    }

    // Try adding the node to subset X if it doesn't exceed the allowed size
    if (x_count < a) {
        partition[node] = 0;
        #pragma omp task firstprivate(partition)
        dfs(node + 1, x_count + 1, cut_weight + delta_y, partition);
    }

    partition[node] = 1; // Assign the node to subset Y and continue the search
    #pragma omp task if (node <= a) firstprivate(partition)
    dfs(node + 1, x_count, cut_weight + delta_x, partition);
}

void CSolver::solve() {
    vector<int> partition(n, -1); // Partition of the nodes
    #pragma omp parallel num_threads(omp_get_max_threads())
    {
        #pragma omp single
        {
            if (2 * a == n) {
                partition[0] = 0;
                // If X and Y must be equal in size, placing the first node in X avoids duplicate results
                dfs(1, 1, 0, partition); // Start DFS with one node already in X
            } else {
                dfs(0, 0, 0, partition); // Otherwise, start DFS normally from the beginning
            }
        }
    }
    print_solution();
}

void CSolver::print_solution() const {
    cout << "Min Cut weight: " << min_cut_weight << endl;
    for (const auto &it: best_partitions) {
        cout << "X: ";
        for (int j = 0; j < n; j++) if (it[j] == 0) cout << j << " ";
        cout << endl << "Y: ";
        for (int j = 0; j < n; j++) if (it[j] == 1) cout << j << " ";
        cout << endl;
    }
    cout << "Recursion: " << n_rec << endl;
}
