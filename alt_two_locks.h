#pragma once

#include <mutex>
#include <shared_mutex>
#include <stdexcept>
#include <string>

using namespace std;

class TwoFieldsPerFieldLocks
{
public:
    TwoFieldsPerFieldLocks() = default;
    TwoFieldsPerFieldLocks(const TwoFieldsPerFieldLocks &) = delete;
    TwoFieldsPerFieldLocks &operator=(const TwoFieldsPerFieldLocks &) = delete;
    ~TwoFieldsPerFieldLocks() = default;

    int get(int idx) const
    {
        check(idx);
        shared_lock lk(idx == 0 ? mtx0 : mtx1);
        return values[idx];
    }

    void set(int idx, int value)
    {
        check(idx);
        lock_guard lk(idx == 0 ? mtx0 : mtx1);
        values[idx] = value;
    }

    operator string() const
    {
        int v0, v1;
        {
            shared_lock lk0(mtx0, defer_lock);
            shared_lock lk1(mtx1, defer_lock);
            lock(lk0, lk1);
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
    mutable shared_mutex mtx0;
    mutable shared_mutex mtx1;
};
