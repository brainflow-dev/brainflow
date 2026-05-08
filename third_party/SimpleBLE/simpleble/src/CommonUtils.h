#pragma once

#include <algorithm>
#include <vector>

#include "LoggingInternal.h"

#define SAFE_CALLBACK_CALL(cb, ...)                                                           \
    do {                                                                                      \
        if (cb) {                                                                             \
            try {                                                                             \
                cb(__VA_ARGS__);                                                              \
            } catch (const std::exception& ex) {                                              \
                SIMPLEBLE_LOG_ERROR(fmt::format("Exception during callback: {}", ex.what())); \
            } catch (...) {                                                                   \
                SIMPLEBLE_LOG_ERROR("Unknown exception during callback");                     \
            }                                                                                 \
        }                                                                                     \
    } while (0)

#define SAFE_RUN(code)                                                                      \
    do {                                                                                    \
        try {                                                                               \
            {                                                                               \
                code                                                                        \
            }                                                                               \
        } catch (const std::exception& ex) {                                                \
            SIMPLEBLE_LOG_ERROR(fmt::format("Exception within code block: {}", ex.what())); \
        } catch (...) {                                                                     \
            SIMPLEBLE_LOG_ERROR("Unknown exception within code block");                     \
        }                                                                                   \
    } while (0)

namespace SimpleBLE::Util {

template <typename MAP>
struct ValueCollector {
  public:
    template <typename VEC>
    operator VEC() {
        VEC vec;
        vec.reserve(map.size());
        std::transform(map.begin(), map.end(), std::back_inserter(vec), [](const auto& pair) { return pair.second; });
        return vec;
    }

    const MAP& map;
};

/**
 * Collect the values of a mapping like object into a vector-like object.
 */
template <typename MAP>
auto values(const MAP& map) {
    return ValueCollector<MAP>{map};
}

}  // namespace SimpleBLE::Util

namespace SimpleBLE {
template <typename T>
using SharedPtrVector = std::vector<std::shared_ptr<T>>;
}
