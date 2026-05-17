/**
 * @file visualiser.hh
 * @author Bartosz ksel (275472@student.pwr.edu.pl)
 * @brief Visualizations methods.
 * @version 0.1
 * @date 2026-05-17
 *
 * @copyright Copyright (c) 2026
 *
 */

#pragma once

#include <iomanip>
#include <iostream>

#include "grid_world.hh"

namespace mdp {

// Helper to print the optimal policy in a grid
inline void print_policy(const mdp::GridWorld& world)
{
    auto config = world.getConfig();
    std::cout << "\nPolicy:\n";

    // Create the horizontal border for the policy grid
    std::string hline = "+";
    for (int i = 0; i < config.width; ++i) {
        hline += "---+";
    }

    for (int y = config.height; y >= 1; --y) {
        std::cout << hline << "\n|";
        for (int x = 1; x <= config.width; ++x) {
            auto state = world.getState(x, y);
            if (state->type == mdp::EState::Forbidden)
                std::cout << " F |";
            else if (state->type == mdp::EState::Terminal)
                std::cout << " T |";
            else
                std::cout << " " << state->policy << " |";
        }
        std::cout << "\n";
    }
    std::cout << hline << "\n";
}

inline void print_utilities(const mdp::GridWorld& world)
{
    auto config = world.getConfig();
    std::cout << "\nUtilities:\n";

    // Create a wider horizontal border for 4-decimal floating point numbers
    std::string hline = "+";
    for (int i = 0; i < config.width; ++i) {
        hline += "----------+";
    }

    std::cout << std::fixed << std::setprecision(4);
    for (int y = config.height; y >= 1; --y) {
        std::cout << hline << "\n|";
        for (int x = 1; x <= config.width; ++x) {
            auto state = world.getState(x, y);
            if (state->type == mdp::EState::Forbidden)
                std::cout << "    F     |";
            else
                std::cout << " " << std::setw(8) << state->utility << " |";
        }
        std::cout << "\n";
    }
    std::cout << hline << "\n";
}
} // namespace mdp