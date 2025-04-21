#include "CSolver.h"
#include <algorithm>
#include <iostream>
#include <limits>
#include <queue>
#include <cmath>
#include <map>
#include <omp.h>
#include <mpi.h>

enum MPI_TAGS {
    STATE = 1,
    MIN_CUT = 2,
    TERMINATE = 3,
    WAITING = 4,
    RESULT = 5
};

CSolver::CSolver(const int n, const int a, const vector<vector<int> > &graph)
    : n(n), a(a), graph(graph), min_cut_weight(numeric_limits<int>::max()) {
}

queue<CState> CSolver::starting_states(const int depth) const {
    queue<CState> result;
    queue<CState> q;

    if (2 * a == n) {
        CState new_state(1, 1, 0);
        new_state.partition[0] = 0;
        q.emplace(new_state);
    } else {
        CState new_state;
        q.emplace(new_state);
    }

    while (!q.empty()) {
        CState state = q.front();
        q.pop();

        if (state.node == depth) {
            result.push(state);
            continue;
        }

        const int node = state.node;

        int delta_x = 0, delta_y = 0; // Contribution to cut weight when assigning node to X or Y
        for (int i = 0; i < node; i++) {
            if (state.partition[i] == 0)
                delta_x += graph[i][node]; // Adding edges between node and subset X
            else
                delta_y += graph[i][node]; // Adding edges between node and subset Y
        }

        if (state.x_count < a) {
            // Add the node to subset X
            CState left = CState(node + 1, state.x_count + 1, state.cut_weight + delta_y, state.partition);
            left.partition[node] = 0;
            q.push(left);
        }

        if (state.node - state.x_count < n - a) {
            // Add the node to subset Y
            CState right = CState(node + 1, state.x_count, state.cut_weight + delta_x, state.partition);
            right.partition[node] = 1;
            q.push(right);
        }
    }

    return result;
}

void CSolver::master(const int num_procs) {
    queue<CState> states = starting_states(a - 1);
    const int num_slaves = num_procs - 1;
    send_configurations(states, num_slaves);

    vector<int> results(num_slaves, 0); // num_results
    int cut_weight;

    int active_slaves = num_slaves;
    while (active_slaves > 0) {
        MPI_Status status;
        MPI_Recv(&cut_weight, 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
        const int slave_id = status.MPI_SOURCE;

        if (status.MPI_TAG == MIN_CUT) {
            if (cut_weight == min_cut_weight) {
                results[slave_id - 1]++;
            } else if (cut_weight < min_cut_weight) {
                min_cut_weight = cut_weight;
                results.clear();
                results.resize(num_slaves, 0);
                results[slave_id - 1]++;
            }
        } else if (status.MPI_TAG == WAITING) {
            if (!states.empty()) {
                CState next_state = states.front();
                next_state.min_cut_weight = min_cut_weight;
                MPI_Send(&next_state, sizeof(CState), MPI_BYTE, slave_id, STATE, MPI_COMM_WORLD);
                states.pop();
            } else {
                active_slaves--;
            }
        }
    }

    vector<vector<int> > result;
    for (int i = 1; i <= num_slaves; i++) {
        const int num_results = results[i - 1];
        if (num_results > 0) {
            MPI_Send(nullptr, 0, MPI_BYTE, i, RESULT, MPI_COMM_WORLD);
            vector<int> flat_buffer(num_results * n);
            MPI_Recv(flat_buffer.data(), num_results * n, MPI_INT, i, RESULT, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            for (int j = 0; j < num_results; ++j) {
                vector<int> partition(flat_buffer.begin() + j * n, flat_buffer.begin() + (j + 1) * n);
                result.push_back(partition);
            }
        } else {
            MPI_Send(nullptr, 0, MPI_BYTE, i, TERMINATE, MPI_COMM_WORLD);
        }
    }

    print_solution(min_cut_weight, result);
}

void CSolver::send_configurations(queue<CState> &states, const int num_slaves) {
    for (int i = 1; i <= num_slaves; i++) {
        CState state = states.front();
        MPI_Send(&state, sizeof(CState), MPI_BYTE, i, STATE, MPI_COMM_WORLD);
        states.pop();
    }
}

void CSolver::solve() {
    CState state;
    MPI_Recv(&state, sizeof(CState), MPI_BYTE, 0, STATE, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

    while (true) {
        vector<int> partition(state.partition, state.partition + n);
        if (min_cut_weight > state.min_cut_weight) {
            best_partitions.clear();
            min_cut_weight = state.min_cut_weight;
        }

#pragma omp parallel num_threads(omp_get_max_threads())
        {
#pragma omp single
            {
                dfs(state.node, state.x_count, state.cut_weight, partition);
            }
        }

        MPI_Send(&min_cut_weight, 1, MPI_INT, 0, WAITING, MPI_COMM_WORLD);

        MPI_Status status;
        MPI_Recv(&state, sizeof(CState), MPI_BYTE, 0, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
        if (status.MPI_TAG == TERMINATE) {
            return;
        } else if (status.MPI_TAG == RESULT) {
            vector<int> flat_buffer;
            const int num_results = static_cast<int>(best_partitions.size());
            flat_buffer.reserve(num_results * n);
            for (const auto &p: best_partitions) {
                flat_buffer.insert(flat_buffer.end(), p.begin(), p.end());
            }
            MPI_Send(flat_buffer.data(), num_results * n, MPI_INT, 0, RESULT, MPI_COMM_WORLD);
            return; // TERMINATE
        }
    }
}

void CSolver::dfs(const int node, const int x_count, const int cut_weight, vector<int> &partition) {
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
                    // Send results back to the master
                    MPI_Send(&min_cut_weight, 1, MPI_INT, 0, MIN_CUT, MPI_COMM_WORLD);
                } else if (cut_weight < min_cut_weight) {
                    min_cut_weight = cut_weight; // Update the best cut weight found
                    best_partitions.clear(); // Clear previous partitions
                    best_partitions.push_back(partition); // Store the new best partition
                    // Send results back to the master
                    MPI_Send(&min_cut_weight, 1, MPI_INT, 0, MIN_CUT, MPI_COMM_WORLD);
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
#pragma omp task firstprivate(partition)
    dfs(node + 1, x_count, cut_weight + delta_x, partition);
}

void CSolver::print_solution(const int cut_weight, const vector<vector<int> > &partition) const {
    cout << "Min Cut weight: " << cut_weight << endl;
    for (const auto &it: partition) {
        cout << "X: ";
        for (int j = 0; j < n; j++) if (it[j] == 0) cout << j << " ";
        cout << endl << "Y: ";
        for (int j = 0; j < n; j++) if (it[j] == 1) cout << j << " ";
        cout << endl;
    }
    cout << endl;
}
