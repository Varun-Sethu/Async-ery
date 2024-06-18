#pragma once

#include <atomic>


template <typename SpinLock>
class SpinLockGuard {
    public:
        SpinLockGuard(SpinLock& lock) : lock(lock) {}
        ~SpinLockGuard() { lock.unlock(); }

        // Guards cannot be transferred
        SpinLockGuard(const SpinLockGuard&) = delete;
        SpinLockGuard& operator=(const SpinLockGuard&) = delete;
        SpinLockGuard(SpinLockGuard&&) = delete;
        SpinLockGuard& operator=(SpinLockGuard&&) = delete;

    private:
        SpinLock& lock;
};

class SpinLock {
    public:
        auto lock() -> SpinLockGuard<SpinLock>;
        auto unlock() -> void;

    private:
        std::atomic<bool> is_acquired = { false };
};