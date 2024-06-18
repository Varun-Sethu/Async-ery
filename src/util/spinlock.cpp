#include "util/spinlock.h"

auto SpinLock::lock() -> SpinLockGuard<SpinLock> {
    for (;;) {
        if (!is_acquired.exchange(true, std::memory_order_acquire)) { break; }
        while (is_acquired.load(std::memory_order_relaxed)) { __builtin_ia32_pause(); }
    }

    return SpinLockGuard(*this);
}

auto SpinLock::unlock() -> void { is_acquired.store(false, std::memory_order_release); }