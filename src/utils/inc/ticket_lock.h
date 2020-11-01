#pragma once

#include <atomic>


class TicketLock
{
public:
    inline void lock ()
    {
        const auto ticket_num = next_ticket_num.fetch_add (1, std::memory_order_relaxed);
        while (serving_ticket_num.load (std::memory_order_acquire) != ticket_num)
        {
        }
    }

    inline void unlock ()
    {
        const auto new_num = serving_ticket_num.load (std::memory_order_relaxed) + 1;
        serving_ticket_num.store (new_num, std::memory_order_release);
    }

private:
    std::atomic_size_t serving_ticket_num = {0};
    std::atomic_size_t next_ticket_num = {0};
};
