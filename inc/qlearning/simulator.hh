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

#include "defs.hh"
#include "fast_rng.hh"

namespace ql {
class Simulator {
private:
    std::vector<FastState> grid;
    int padded_width;
    double p1, p2, p3;

public:
    Simulator(int raw_width, int raw_height, double p1, double p2, double p3)
      : padded_width(raw_width + 2)
      , p1(p1)
      , p2(p2)
      , p3(p3)
    {
        // Initialize padded grid (raw_width + 2) x (raw_height + 2)
        grid.assign(padded_width * (raw_height + 2), {EState::Forbidden, 0.0});

        // TODO: Fill the inner playable area with your actual parsed file data!
        // grid[y * padded_width + x] = parsed_state;
    }

    // Notice: NO bounds checking needed because of the Forbidden padding!
    StepResult execute_action(int curr_x, int curr_y, EAction action, FastRNG& rng) const noexcept
    {
        // 1. TODO: Roll rng.next_double() and apply p1, p2, p3 to determine actual_move
        EAction actual_move = action; // Placeholder

        // 2. Calculate next coordinates based on actual_move
        int nx = curr_x;
        int ny = curr_y;
        // if (actual_move == EAction::Up) ny += 1; etc...

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
} // namespace qlearning