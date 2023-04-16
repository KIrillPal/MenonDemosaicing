#pragma once
#include <cstdint>
#include <vector>

namespace menon {
    using FIRFilter = std::vector<int>;
    enum Direction {
        HORIZONTAL = 0,
        VERTICAL   = 1,
    };
} // namespace menon
