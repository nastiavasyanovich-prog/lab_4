#pragma once

#include <mutex>
#include <shared_mutex>
#include <stdexcept>
#include <string>

using namespace std;

class TwoFields
{
public:
    TwoFields() = default;
    TwoFields(const TwoFields &) = delete;
    TwoFields &operator=(const TwoFields &) = delete;
    ~TwoFields() = default;

    int get(int idx) const
    {
        check(idx);
        shared_lock lk(mtx);
        return values[idx];
    }

    void set(int idx, int value)
    {
        check(idx);
        lock_guard lk(mtx);
        values[idx] = value;
    }

    operator string() const
    {
        int v0, v1;
        {
            shared_lock lk(mtx);
            v0 = values[0];
            v1 = values[1];
        }
        return "[" + to_string(v0) + ", " + to_string(v1) + "]";
    }

private:
    static void check(int idx)
    {
        if (idx < 0 || idx > 1)
            throw out_of_range("field index must be 0 or 1");
    }

    int values[2] = {0, 0};
    mutable shared_mutex mtx;
};
