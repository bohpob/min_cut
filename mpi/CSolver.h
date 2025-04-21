#pragma once
#include <queue>
#include <vector>
#include "CState.h"

using namespace std;

class CSolver {
    const int n, a;
    const vector<vector<int> > graph;

    int min_cut_weight; // Weight of the minimum cut
    vector<vector<int> > best_partitions; // Best solutions

public:
    CSolver(int n, int a, const vector<vector<int> > &graph);

    void dfs(int node, int x_count, int cut_weight, vector<int> &partition);

    queue<CState> starting_states(int depth) const;

    void send_configurations(queue<CState> &states, int num_slaves);

    void master(int num_procs);

    void solve();

    void print_solution(int cut_weight, const vector<vector<int>> &partition) const;
};
