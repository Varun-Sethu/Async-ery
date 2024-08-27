#pragma once

#include <atomic>

class SpinLock {
public:
    auto lock() -> void;
    auto unlock() -> void;

private:
    std::atomic<bool> is_acquired = { false };
};