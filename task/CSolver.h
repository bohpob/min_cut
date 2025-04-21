#pragma once
#include <vector>

using namespace std;

class CSolver {
    const int n, a;
    const vector<vector<int> > graph;

    int min_cut_weight; // Weight of the minimum cut
    vector<vector<int> > best_partitions; // Best solutions

public:
    CSolver(int n, int a, const vector<vector<int> > &graph);

    void dfs(int node, int x_count, int cut_weight, vector<int> &partition);

    void solve();

    void print_solution() const;
};
