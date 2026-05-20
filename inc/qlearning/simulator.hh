/**
 * @file simulator.hh
 * @author Bartosz ksel (275472@student.pwr.edu.pl)
 * @brief Simulator for Q-learning process.
 * @version 0.1
 * @date 2026-05-16
 *
 * @copyright Copyright (c) 2026
 *
 */

#pragma once

#include <vector>

#include "core_optional.hh"

#include "defs.hh"
#include "fast_rng.hh"

#include "mdp/common_defs.hh"
#include "mdp/grid_world.hh"

namespace ql {
class Simulator {
private:
    std::vector<FastState> grid;
    std::size_t padded_width;
    double p1, p2, p3;

    Simulator(
      int raw_width,
      int raw_height,
      double p1,
      double p2,
      double p3
    )
      : padded_width(raw_width + 2)
      , p1(p1)
      , p2(p2)
      , p3(p3)
    {
        // Initialize padded grid (raw_width + 2) x (raw_height + 2)
        grid.assign(padded_width * (raw_height + 2), {EState::Forbidden, 0.0});
    }

public:
    static core::optional<Simulator>
    from_gridworld(const mdp::GridWorld& world)
    {
        auto config = world.getConfig();
        if (config.width <= 0 || config.height <= 0) {
            return core::nullopt;
        }

        // Use the private constructor to allocate the padded grid
        Simulator sim(config.width, config.height, config.p1, config.p2, config.p3);

        // Copy states from GridWorld into the Simulator's high-speed grid
        for (int y = 1; y <= config.height; ++y) {
            for (int x = 1; x <= config.width; ++x) {
                auto state_opt = world.getState(x, y);
                if (!state_opt) continue;

                const auto& state = *state_opt;

                // Map mdp::StateType to ql::EState
                EState fast_type = state.type;

                // x and y naturally start at 1, which perfectly skips the x=0 and y=0 padding!
                sim.grid[y * sim.padded_width + x] = {fast_type, state.reward};
            }
        }

        return sim;
    }

    // Notice: NO bounds checking needed because of the Forbidden padding!
    StepResult
    execute_action(
      std::size_t curr_x,
      std::size_t curr_y,
      EAction action,
      FastRNG& rng
    ) const noexcept
    {
        // 1. Roll rng and determine actual stochastic movement direction
        double roll = rng.next_double();
        EAction actual_move = action;

        if (roll < p1) [[likely]] {
            actual_move = action;
        } else if (roll < p1 + p2) {
            actual_move = mdp::get_left_action(action);
        } else if (roll < p1 + p2 + p3) {
            actual_move = mdp::get_right_action(action);
        } else [[unlikely]] {
            actual_move = mdp::get_opposite_action(action);
        }

        // 2. Fetch signed directional deltas from common mdp namespace
        auto [dx, dy] = mdp::get_dir(actual_move);

        // Safe unsigned coordinate arithmetic to prevent implicit negative promotion traps
        std::size_t nx = (dx >= 0) ? (curr_x + static_cast<std::size_t>(dx))
                                   : (curr_x - static_cast<std::size_t>(-dx));
        std::size_t ny = (dy >= 0) ? (curr_y + static_cast<std::size_t>(dy))
                                   : (curr_y - static_cast<std::size_t>(-dy));

        // 3. Fast 1D lookup
        const auto& next_state = grid[ny * padded_width + nx];

        // 4. Ghost cell / Wall collision
        if (next_state.type == EState::Forbidden) {
            const auto& current_state = grid[curr_y * padded_width + curr_x];
            return {curr_x, curr_y, current_state.reward, false};
        }

        return {nx, ny, next_state.reward, next_state.type == EState::Terminal};
    }
};
} // namespace ql