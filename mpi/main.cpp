#include <iostream>
#include <vector>
#include <chrono>
#include <mpi.h>

#include "CFileReader.h"
#include "CSolver.h"

using namespace std;
using namespace chrono;

int main(int argc, char *argv[]) {
    if (argc != 3) {
        cerr << "<Size of set X = 'a'> <input_file>" << endl;
        return 1;
    }

    int my_rank, num_procs;
    /* Initialize MPI */
    MPI_Init(&argc, &argv);
    /* find out process rank */
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    /* find out number of processes */
    MPI_Comm_size(MPI_COMM_WORLD, &num_procs);

    // Start measuring execution time
    time_point<high_resolution_clock> start_time;
    if (my_rank == 0) {
        start_time = high_resolution_clock::now();
    }

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

    // Master-Slave approach
    if (my_rank == 0) {
        // Master process generates initial configurations
        CSolver solver(n, a, graph);
        solver.master(num_procs);
    } else {
        // Slave processes receive configurations and perform DFS
        CSolver solver(n, a, graph);
        solver.solve();
    }

    // Measure and display execution time
    if (my_rank == 0) {
        const auto end_time = high_resolution_clock::now();
        const duration<double> elapsed = end_time - start_time;
        cout << "Time: " << elapsed.count() << " seconds" << endl;
    }

    MPI_Finalize();
    return 0;
}
