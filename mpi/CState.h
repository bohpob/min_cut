#pragma once
#include <limits>

constexpr int PARTITION_SIZE = 40;

struct CState {
    int node;
    int x_count;
    int cut_weight;
    int min_cut_weight;
    int partition[PARTITION_SIZE];

    CState() : node(0), x_count(0), cut_weight(0), min_cut_weight(std::numeric_limits<int>::max()) {
        std::fill(partition, partition + PARTITION_SIZE, -1);
    }

    CState(const int node, const int x_count, const int cut_weight)
        : node(node), x_count(x_count), cut_weight(cut_weight), min_cut_weight(std::numeric_limits<int>::max()) {
        std::fill(partition, partition + PARTITION_SIZE, -1);
    }

    CState(const int node, const int x_count, const int cut_weight, const int *partition_data)
        : node(node), x_count(x_count), cut_weight(cut_weight), min_cut_weight(std::numeric_limits<int>::max()) {
        std::copy(partition_data, partition_data + PARTITION_SIZE, partition);
    }
};
