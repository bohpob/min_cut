#pragma once
#include <vector>
#include <string>

using namespace std;

class CFileReader {
public:
    bool readInteger(int &i, const string &argv);

    bool readFromFile(const string &filename, vector<vector<int> > &graph, int &n);
};
