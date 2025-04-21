#pragma once
#include <vector>

using namespace std;

struct CState {
    int node = 0;
    int x_count = 0;
    int cut_weight = 0;
    vector<int> partition{};

    CState() = default;

    CState(const int node, const int x_count, const int cut_weight, const vector<int> &partition)
        : node(node), x_count(x_count), cut_weight(cut_weight), partition(partition) {
    }
};

class CSolver {
    const int n, a;
    const vector<vector<int> > graph;

    int min_cut_weight; // Weight of the minimum cut
    vector<vector<int> > best_partitions; // Best solutions

public:
    CSolver(int n, int a, const vector<vector<int> > &graph);

    void dfs(int node, int x_count, int cut_weight, vector<int> &partition);

    vector<CState> starting_states() const;

    void solve();

    void print_solution() const;
};
