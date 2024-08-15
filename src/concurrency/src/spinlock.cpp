#include <time.h>
#include <iostream>

#include "concurrency/spinlock.h"

auto SpinLock::lock() -> void {
    const int num_attempts = 15;
    const timespec backoff_time = {0, 1};

    for (;;) {
        // try to acquire the mutex (num_attempts) times, eventually backing off and yielding
        auto potentially_acquirable = false;
        for (int i = 0; i < num_attempts; i++) {
            if (!is_acquired.load(std::memory_order_relaxed)) { potentially_acquirable = true; break; }
            __builtin_ia32_pause();
        }
        
        if (potentially_acquirable && !is_acquired.exchange(true, std::memory_order_acquire)) { return; }
        else {
            // back off and yield
            nanosleep(&backoff_time, nullptr);
        }
    }
}

auto SpinLock::unlock() -> void { is_acquired.store(false, std::memory_order_release); }