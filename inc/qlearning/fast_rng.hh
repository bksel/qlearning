/**
 * @file fast_rng.hh
 * @author Bartosz ksel (275472@student.pwr.edu.pl)
 * @brief Fast RNG for Q-learing,
 * @version 0.1
 * @date 2026-05-16
 *
 * @copyright Copyright (c) 2026
 *
 */
#pragma once
#include <cstdint>

namespace ql {

struct FastRNG {
    uint64_t state;
    FastRNG(uint64_t seed)
      : state(seed == 0 ? 1 : seed)
    {
    }

    inline uint32_t
    next() noexcept
    {
        state ^= state << 13;
        state ^= state >> 7;
        state ^= state << 17;
        return static_cast<uint32_t>(state);
    }

    inline double
    next_double() noexcept
    {
        return (next() >> 8) * 0x1.0p-24;
    }
};
} // namespace ql