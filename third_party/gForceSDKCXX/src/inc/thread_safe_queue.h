#pragma once

#include <condition_variable>
#include <mutex>
#include <queue>


template <class T> class ThreadSafeQueue
{
public:
    ThreadSafeQueue ()
    {
    }
    ~ThreadSafeQueue ()
    {
    }
    void push (T val)
    {
        std::lock_guard<std::mutex> lgx (mQueueMx);
        mDataQueue.push (val);
        mQueueCv.notify_one ();
    }

    T wait_and_pop ()
    {
        std::unique_lock<std::mutex> lk (mQueueMx);
        mQueueCv.wait (lk, [this] { return !mDataQueue.empty (); });
        T retval = mDataQueue.front ();
        mDataQueue.pop ();
        return retval;
    }
    bool empty ()
    {
        std::lock_guard (mQueueMx);
        return mDataQueue.empty ();
    }

private:
    std::queue<T> mDataQueue;
    std::mutex mQueueMx;
    std::condition_variable mQueueCv;
};