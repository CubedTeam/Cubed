#pragma once
#include "Cubed/gameplay/world.hpp"
#include "Cubed/tools/cubed_assert.hpp"

#include <algorithm>
#include <thread>
namespace Cubed {
namespace Tools {

constexpr size_t SERVER_RESERVED_THREADS = 3; // tick + netio + gen scheduler
constexpr size_t CLIENT_RESERVED_THREADS =
    3; // main/render + netio + system reserved

constexpr size_t safe_sub(size_t a, size_t b) { return a > b ? a - b : 0; }

inline size_t get_hardware_threads() {
    auto hc = std::thread::hardware_concurrency();
    return hc == 0 ? 4 : static_cast<size_t>(hc);
}

inline size_t get_server_available_threads() {
    return std::max<size_t>(
        1, safe_sub(get_hardware_threads(), SERVER_RESERVED_THREADS));
}

inline size_t get_client_available_threads() {
    return std::max<size_t>(
        1, safe_sub(get_hardware_threads(), CLIENT_RESERVED_THREADS));
}

inline size_t get_client_threads(RunMode mode) {
    switch (mode) {
    case RunMode::SERVER_ONLY:
        ASSERT_MSG(false, "Server Only don't need client pool");
        return 1;
    case RunMode::CLIENT_ONLY: {
        auto available = get_client_available_threads();
        return std::clamp<size_t>(available, 1, 16);
    }

    case RunMode::HYBRID: {
        auto available = get_client_available_threads();
        return std::clamp<size_t>(available / 2, 1, 4);
    }
    }
    return 1;
}

inline size_t get_server_net_pool_threads(RunMode mode) {
    switch (mode) {

    case RunMode::SERVER_ONLY: {
        auto available = get_server_available_threads();
        return std::clamp<size_t>(available / 4, 1,
                                  std::min<size_t>(4, available));
    }

    case RunMode::CLIENT_ONLY:
        ASSERT_MSG(false, "Client Only don't need net pool");
        return 1;
    case RunMode::HYBRID: {
        auto available = get_server_available_threads();
        return std::clamp<size_t>(available / 8, 1,
                                  std::min<size_t>(4, available));
    }
    }
    return 1;
}

inline size_t get_server_compute_treads(RunMode mode) {
    switch (mode) {
    case RunMode::HYBRID:
    case RunMode::SERVER_ONLY: {
        auto available = get_server_available_threads();

        return std::clamp<size_t>(available / 4, 1,
                                  std::min<size_t>(4, available));
    }
    case RunMode::CLIENT_ONLY:
        ASSERT_MSG(false, "Client Only don't need update pool");
        return 1;
    }
    return 1;
}

inline size_t get_server_gen_threads(RunMode mode) {
    switch (mode) {
    case RunMode::SERVER_ONLY: {
        auto available = get_server_available_threads();

        auto net_pool = get_server_net_pool_threads(mode);

        auto update_pool = get_server_compute_treads(mode);

        size_t remain = available;

        remain -= std::min(remain, net_pool);

        remain -= std::min(remain, update_pool);

        return std::max<size_t>(1, remain);
    }
    case RunMode::CLIENT_ONLY:
        ASSERT_MSG(false, "Client Only don't need gen pool");
        return 1;
    case RunMode::HYBRID: {
        auto available = get_server_available_threads();

        auto net_pool = get_server_net_pool_threads(mode);

        auto update_pool = get_server_compute_treads(mode);

        auto client_pool = get_client_threads(mode);

        size_t remain = available;

        remain -= std::min(remain, net_pool);

        remain -= std::min(remain, update_pool);

        remain -= std::min(remain, client_pool);

        return std::max<size_t>(1, remain);
    }
    }
    return 1;
}

} // namespace Tools
} // namespace Cubed
