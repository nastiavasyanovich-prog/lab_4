#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <random>
#include <string>
#include <vector>

using namespace std;

struct Scenario
{
    char name;
    int percent[5];
};

const char *commands[5] = {"read 0", "write 0", "read 1", "write 1", "string"};

const Scenario scenarios[3] = {
    {'a', {10, 5, 50, 5, 30}},
    {'b', {20, 20, 20, 20, 20}},
    {'c', {5, 40, 5, 40, 10}},
};

void writeFile(const string &path, const Scenario &sc, int count, mt19937 &rng)
{
    vector<int> kinds;
    for (int k = 0; k < 5; ++k)
    {
        int n = count / 100 * sc.percent[k];
        for (int i = 0; i < n; ++i)
            kinds.push_back(k);
    }
    shuffle(kinds.begin(), kinds.end(), rng);

    uniform_int_distribution<int> value(0, 99);
    ofstream f(path);
    for (int k : kinds)
    {
        f << commands[k];
        if (k == 1 || k == 3)
            f << ' ' << value(rng);
        f << '\n';
    }
}

int main(int argc, char *argv[])
{
    string dir = argc > 1 ? argv[1] : "data";
    int total = argc > 2 ? stoi(argv[2]) : 1200000;

    if (total <= 0 || total % 600 != 0)
    {
        cerr << "N must be a positive multiple of 600\n";
        return 1;
    }

    filesystem::create_directories(dir);
    mt19937 rng(20);
    for (const Scenario &sc : scenarios)
    {
        for (int threads = 1; threads <= 3; ++threads)
        {
            for (int i = 0; i < threads; ++i)
            {
                string path = dir + "/" + sc.name + "_" + to_string(threads) + "t_" +
                              to_string(i) + ".txt";
                writeFile(path, sc, total / threads, rng);
                cout << "created " << path << " (" << total / threads << " ops)\n";
            }
        }
    }
    return 0;
}
