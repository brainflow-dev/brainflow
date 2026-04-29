#pragma once

#include "BuilderBase.h"

namespace SimpleBLE::Factory {

template <typename V1>
struct BuildVecDeduce {
    template <typename T>
    operator T() && {
        T vec;
        vec.reserve(objs.size());
        for (auto& obj : objs) {
            vec.push_back(build(std::move(obj)));
        }
        return vec;
    }

    V1 objs;
};

/**
 * Build a vector of objects which are not directly constructible.
 *
 * This calls `Factory::build()` for each object in the input vector.
 */
template <typename V1>
auto vector(V1 objs) {
    return BuildVecDeduce<V1>{std::move(objs)};
}

}  // namespace SimpleBLE::Factory
