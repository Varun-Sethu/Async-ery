// NOLINTBEGIN
//  Note: this is just a test file to demonstrate how to use the library
//        and is not a part of the library itself.

#include <iostream>
#include <chrono>
#include <ostream>
#include <thread>
#include <memory>

#include "timing/structures/timing_wheel_hierarchical.h"

using std::chrono_literals::operator""ms;

class IntWrapper {
    public:
        IntWrapper(int value) : value(std::make_unique<int>(value)) {}
        auto get() -> int { return *value; }        
        friend std::ostream& operator<<(std::ostream& os, const IntWrapper& value_inner) {
            os << *value_inner.value;
            return os;
        }
    private:
        std::unique_ptr<int> value;
};

auto main() -> int {
    /**
     *  Wheel schematics:
     *      
     *      lvl 0: [0ms -> 200ms)     --- [0ms - 100ms)[100ms - 200ms)                          -> [1][2]
     *      lvl 1: [200ms -> 800ms)   --- [200ms - 400ms)[400ms - 600ms)[600ms - 800ms)         -> [3-4][4-5][6-7]
     *      lvl 2: [800ms -> 2000ms)  --- [800ms - 1400ms)[1400ms - 2000ms)                     -> [8-14][14-20]
     * 
     */
    auto wheel = Timing::HierarchicalTimingWheel<IntWrapper>(100ms, {2, 3, 2});

    wheel.schedule(0ms, IntWrapper(1));
    wheel.schedule(50ms, IntWrapper(2));
    wheel.schedule(150ms, IntWrapper(3));
    wheel.schedule(250ms, IntWrapper(4));
    wheel.schedule(310ms, IntWrapper(5));
    wheel.schedule(650ms, IntWrapper(6));
    wheel.schedule(750ms, IntWrapper(7));
    wheel.schedule(1000ms, IntWrapper(8));
    wheel.schedule(1950ms, IntWrapper(9));
    wheel.schedule(800ms, IntWrapper(10));

    for (int i = 0; i < 10; i++) {
        std::this_thread::sleep_for(200ms);
        wheel.schedule(500ms, IntWrapper(10 + i));
        for (auto& timer : wheel.advance()) {
            std::cout << "Resolved timer: " << timer << std::endl;
        }

        std::cout << "===" << std::endl;
    }

    std::this_thread::sleep_for(500ms);
    for (auto& timer : wheel.advance()) {
        std::cout << "Resolved timer: " << timer << std::endl;
    }

    return 0;
}


// NOLINTEND