#pragma once

// Prevent unsigned underflow issues in subtraction
#include "Cubed/tools/cubed_assert.hpp"

#include <functional>
#include <utility>
namespace Cubed {
using TickType = long long;

constexpr int DEFAULT_PER_TICK_TIME = 50;

constexpr TickType DAY_TIME = 24000;

constexpr TickType PER_HOUR = 1000;

class Timer {
public:
    template <typename Fn>
    Timer(TickType threshold, Fn&& f)
        : m_fn(std::forward<Fn>(f)), m_threshold(threshold) {
        ASSERT_MSG(threshold > 0, "Threshold Must Rreater Than 0");
    }
    bool update() {
        if (++m_current >= m_threshold) {
            m_current = 0;
            m_fn();
            return true;
        }
        return false;
    }
    void reset() { m_current = 0; }

private:
    std::function<void()> m_fn;
    TickType m_threshold;
    TickType m_current = 0;
};

} // namespace Cubed
