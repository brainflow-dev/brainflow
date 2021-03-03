#pragma once

#include <deque>
#include <set>

template <typename T>
class RollingFilter
{

protected:
    int period;

public:
    RollingFilter (int period)
    {
        this->period = period;
    }

    virtual ~RollingFilter ()
    {
    }

    virtual void add_data (T num) = 0;
    virtual T get_value () = 0;
};

template <typename T>
class RollingMedian : public RollingFilter<T>
{

private:
    std::multiset<T> dataset;
    std::deque<T> deque;

public:
    RollingMedian (int period) : RollingFilter<T> (period)
    {
    }

    void add_data (T num)
    {
        this->deque.push_back (num);
        this->dataset.insert (num);
    }

    // https://leetcode.com/problems/sliding-window-median/
    T get_value ()
    {
        if ((int)this->dataset.size () < this->period)
        {
            // to simplify algorithm if there are less data just return the last value
            return this->deque.back ();
        }
        auto it1 = this->dataset.begin ();
        auto it2 = it1;
        std::advance (it1, (this->period - 1) / 2);
        it2 = it1;
        std::advance (it2, (int)((this->period & 1) == 0));
        T res = (*it1 + *it2) / 2.0;
        T val = deque.front ();
        deque.pop_front ();
        this->dataset.erase (this->dataset.find (val));
        return res;
    }
};

template <typename T>
class RollingAverage : public RollingFilter<T>
{

private:
    std::deque<T> dataset;
    T sum;

public:
    RollingAverage (int period) : RollingFilter<T> (period)
    {
        this->sum = 0;
    }

    void add_data (T num)
    {
        this->sum += num;
        this->dataset.push_back (num);
        if ((int)this->dataset.size () > this->period)
        {
            this->sum -= this->dataset.front ();
            this->dataset.pop_front ();
        }
    }

    T get_value ()
    {
        if ((int)this->dataset.size () < this->period)
        {
            return this->sum / this->dataset.size ();
        }
        else
        {
            return this->sum / this->period;
        }
    }
};
