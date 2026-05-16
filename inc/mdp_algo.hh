/**
 * @file mdp_algo.hh
 * @author Bartosz ksel (275472@student.pwr.edu.pl)
 * @brief Algorithms for MDP for task 1.
 * @version 0.1
 * @date 2026-05-16
 *
 * @copyright Copyright (c) 2026
 *
 */

#pragma once

#include <cmath>
#include <limits>

#include "common_defs.hh"
#include "grid_world.hh"

namespace mdp {

// Calculates the expected utility for a specific action taking uncertainty into account
inline double calculate_expected_utility(const GridWorld& world, int x, int y, EAction action)
{
    auto config = world.getConfig();
    double p1 = config.p1;
    double p2 = config.p2;
    double p3 = config.p3;
    double p4 = 1.0 - p1 - p2 - p3;

    // Lambda to compute utility of a single directed move
    auto get_u = [&](EAction move_action) {
        auto [dx, dy] = get_dir(move_action);
        int nx = x + dx;
        int ny = y + dy;

        auto next_state = world.getState(nx, ny);

        // "When the agent executes a move outside the wall, or into a forbidden state... she stays
        // in place"
        if (!next_state || next_state->type == StateType::Forbidden) {
            return world.getState(x, y)->utility; // Bounce back
        }
        return next_state->utility;
    };

    double expected = 0.0;
    expected += p1 * get_u(action);                   // Intended direction
    expected += p2 * get_u(get_left_action(action));  // Left of intended
    expected += p3 * get_u(get_right_action(action)); // Right of intended
    if (p4 > 0.0) {
        expected += p4 * get_u(get_opposite_action(action)); // Opposite direction
    }

    return expected;
}

/**
 * @brief Function performing algorithm calculating optimal policies and utilities.
 *
 * @param world
 * @param config
 * @return uint -- number of iterations performed until convergence
 */
inline uint calculate_policies(GridWorld& world, const MDPConfig& config)
{
    // 2. Value Iteration
    double delta;
    uint iteration = 0;

    // TODO: Open file for Gnuplot here (e.g. std::ofstream gnuplot_file("convergence.dat");)
    // even better, pass stream to it

    do {
        delta = 0.0;
        GridWorld next_world = world;

        for (int y = 1; y <= config.height; ++y) {
            for (int x = 1; x <= config.width; ++x) {
                auto state = world.getState(x, y);
                if (!state || state->type == StateType::Forbidden ||
                    state->type == StateType::Terminal)
                    continue; // Skip walls and terminals

                auto max_val = std::numeric_limits<double>::min();
                char best_action = '*';

                for (EAction a : {EAction::Up, EAction::Down, EAction::Left, EAction::Right}) {
                    double expected_u = mdp::calculate_expected_utility(world, x, y, a);
                    if (expected_u > max_val) {
                        max_val = expected_u;
                        best_action = action_to_char(a);
                    }
                }

                auto next_state = next_world.getState(x, y);
                next_state->utility = state->reward + config.gamma.value() * max_val;
                next_state->policy = best_action;

                delta = std::max(delta, std::abs(next_state->utility - state->utility));
            }
        }
        world = next_world;
        iteration++;

        // TODO: Write to gnuplot file -> gnuplot_file << iteration << " " << delta << "\n";

    } while (delta >= 0.0001);

    return iteration;
}
} // namespace mdp