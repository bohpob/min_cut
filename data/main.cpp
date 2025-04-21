#include <iostream>
#include <vector>
#include <chrono>

#include "CFileReader.h"
#include "CSolver.h"

using namespace std;
using namespace chrono;

int main(const int argc, char *argv[]) {
    if (argc != 3) {
        cerr << "<Size of set X = 'a'> <input_file>" << endl;
        return 1;
    }

    // Start measuring execution time
    const auto start_time = high_resolution_clock::now();

    // Read the size of set X
    int a;
    CFileReader file_reader;
    if (!file_reader.readInteger(a, argv[1])) {
        return 1;
    }

    // Read the graph from the input file
    int n;
    vector<vector<int> > graph;
    if (!file_reader.readFromFile(argv[2], graph, n)) {
        return 1;
    }

    // Solve the problem
    CSolver solver(n, a, graph);
    solver.solve();

    // Measure and display execution time
    const auto end_time = high_resolution_clock::now();
    const duration<double> elapsed = end_time - start_time;
    cout << "Time: " << elapsed.count() << " seconds" << endl;
    return 0;
}
