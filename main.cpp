#ifdef ALT_TWO_LOCKS
#include "alt_two_locks.h"
using DataStructure = TwoFieldsPerFieldLocks;
const char *schemeName = "one shared_mutex per field (alternative)";
#else
#include "data_structure.h"
using DataStructure = TwoFields;
const char *schemeName = "one shared_mutex for the whole structure";
#endif

#include <chrono>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <string>
#include <thread>
#include <vector>

using namespace std;

enum class OpType
{
    Read,
    Write,
    String
};

struct Op
{
    OpType type;
    int idx;
    int value;
};

vector<Op> loadOps(const string &path)
{
    ifstream in(path);
    if (!in)
        throw runtime_error("cannot open " + path);

    vector<Op> ops;
    string cmd;
    while (in >> cmd)
    {
        if (cmd == "read")
        {
            int idx;
            in >> idx;
            ops.push_back({OpType::Read, idx, 0});
        }
        else if (cmd == "write")
        {
            int idx, value;
            in >> idx >> value;
            ops.push_back({OpType::Write, idx, value});
        }
        else if (cmd == "string")
        {
            ops.push_back({OpType::String, 0, 0});
        }
        else
        {
            throw runtime_error("bad command '" + cmd + "' in " + path);
        }
    }
    return ops;
}

long long runOps(DataStructure &ds, const vector<Op> &ops)
{
    long long sum = 0;
    for (const Op &op : ops)
    {
        if (op.type == OpType::Read)
            sum += ds.get(op.idx);
        else if (op.type == OpType::Write)
            ds.set(op.idx, op.value);
        else
            sum += string(ds).size();
    }
    return sum;
}

double runThreads(const vector<vector<Op>> &opsPerThread, long long &checksum)
{
    DataStructure ds;
    vector<long long> sums(opsPerThread.size(), 0);

    auto start = chrono::steady_clock::now();
    {
        vector<jthread> threads;
        for (size_t t = 0; t < opsPerThread.size(); ++t)
        {
            threads.emplace_back([&, t]
                                 { sums[t] = runOps(ds, opsPerThread[t]); });
        }
    }
    auto finish = chrono::steady_clock::now();

    checksum = 0;
    for (long long s : sums)
        checksum += s;
    return chrono::duration<double, milli>(finish - start).count();
}

int main(int argc, char *argv[])
{
    string dir = argc > 1 ? argv[1] : "data";
    int repeats = argc > 2 ? stoi(argv[2]) : 10;

    const char scenarios[] = {'a', 'b', 'c'};
    const char *names[] = {
        "a) frequencies match the variant",
        "b) all frequencies equal",
        "c) frequencies strongly differ from the variant",
    };
    double avg[3][3] = {};

    cout << "Synchronization scheme: " << schemeName << "\n\n";

    try
    {
        for (int s = 0; s < 3; ++s)
        {
            cout << "=== Scenario " << names[s] << " ===\n";
            for (int threads = 1; threads <= 3; ++threads)
            {
                vector<vector<Op>> opsPerThread;
                size_t total = 0;
                for (int i = 0; i < threads; ++i)
                {
                    string path = dir + "/" + scenarios[s] + "_" + to_string(threads) +
                                  "t_" + to_string(i) + ".txt";
                    opsPerThread.push_back(loadOps(path));
                    total += opsPerThread.back().size();
                }

                long long checksum = 0;
                runThreads(opsPerThread, checksum);

                double sum = 0;
                cout << "  threads: " << threads << ", ops: " << total << ", runs (ms):";
                for (int r = 0; r < repeats; ++r)
                {
                    double ms = runThreads(opsPerThread, checksum);
                    sum += ms;
                    printf(" %.2f", ms);
                }
                avg[s][threads - 1] = sum / repeats;
                printf("  | average: %.2f ms (checksum %lld)\n", avg[s][threads - 1], checksum);
            }
        }
    }
    catch (const exception &e)
    {
        cerr << "Error: " << e.what() << "\n";
        return 1;
    }

    cout << "\nAverage execution time, ms (" << repeats << " runs):\n";
    printf("%-10s %12s %12s %12s\n", "scenario", "1 thread", "2 threads", "3 threads");
    for (int s = 0; s < 3; ++s)
        printf("%-10c %12.2f %12.2f %12.2f\n", scenarios[s], avg[s][0], avg[s][1], avg[s][2]);

    DataStructure demo;
    demo.set(0, 13);
    demo.set(1, 42);
    cout << "\nDemo: write 0 13; write 1 42; read 1 = " << demo.get(1)
         << "; string = " << string(demo) << "\n";
    return 0;
}
