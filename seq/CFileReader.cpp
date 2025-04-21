#include "CFileReader.h"
#include <fstream>
#include <stdexcept>
#include <iostream>

using namespace std;

bool CFileReader::readInteger(int &i, const string &argv) {
    try {
        i = stoi(argv);
    } catch (const invalid_argument &e) {
        cerr << e.what() << endl;
        return false;
    } catch (const out_of_range &e) {
        cerr << e.what() << endl;
        return false;
    } catch (...) {
        cerr << "Invalid argument: " << argv << endl;
        return false;
    }
    return true;
}

bool CFileReader::readFromFile(const string &filename, vector<vector<int> > &graph, int &n) {
    ifstream file(filename);
    if (!file.is_open()) {
        cerr << "Could not open file: " << filename << endl;
        return false;
    }

    file >> n; // Read the number of nodes

    // Read the graph
    graph.assign(n, vector<int>(n));
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            file >> graph[i][j];
        }
    }
    file.close();
    return true;
}
