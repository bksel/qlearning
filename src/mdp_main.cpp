/**
 * @file mdp_main.cpp
 * @author Bartosz ksel (275472@student.pwr.edu.pl)
 * @brief Entrypoint for program done for 1st task.
 * @version 0.1
 * @date 2026-05-16
 *
 * @copyright Copyright (c) 2026
 *
 */

#include <cmath>
#include <iomanip>
#include <iostream>
#include <limits>

#include "common_defs.hh"
#include "mdp_parser.hh"

enum class EAction { Up, Down, Left, Right };

// Convert action to required character representation
char action_to_char(EAction a)
{
    switch (a) {
    case EAction::Up:
        return '^';
    case EAction::Down:
        return 'v';
    case EAction::Left:
        return '<';
    case EAction::Right:
        return '>';
    }
    return '*';
}

// Helper to get movement deltas
std::pair<int, int> get_dir(EAction a)
{
    switch (a) {
    case EAction::Up:
        return {0, 1};
    case EAction::Down:
        return {0, -1};
    case EAction::Left:
        return {-1, 0};
    case EAction::Right:
        return {1, 0};
    }
    return {0, 0};
}

// Relative direction mappings based on intended action
EAction get_left_action(EAction a)
{
    switch (a) {
    case EAction::Up:
        return EAction::Left;
    case EAction::Down:
        return EAction::Right;
    case EAction::Left:
        return EAction::Down;
    case EAction::Right:
        return EAction::Up;
    }
    return EAction::Up;
}

EAction get_right_action(EAction a)
{
    switch (a) {
    case EAction::Up:
        return EAction::Right;
    case EAction::Down:
        return EAction::Left;
    case EAction::Left:
        return EAction::Up;
    case EAction::Right:
        return EAction::Down;
    }
    return EAction::Up;
}

EAction get_opposite_action(EAction a)
{
    switch (a) {
    case EAction::Up:
        return EAction::Down;
    case EAction::Down:
        return EAction::Up;
    case EAction::Left:
        return EAction::Right;
    case EAction::Right:
        return EAction::Left;
    }
    return EAction::Up;
}

// Calculates the expected utility for a specific action taking uncertainty into account
double calculate_expected_utility(const GridWorld& world, int x, int y, EAction action)
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

// Helper to print grid world results
void print_world(const GridWorld& world)
{
    auto config = world.getConfig();
    std::cout << "\nPolicy:\n";
    for (int y = config.height; y >= 1; --y) {
        for (int x = 1; x <= config.width; ++x) {
            auto state = world.getState(x, y);
            if (state->type == StateType::Forbidden)
                std::cout << " F  ";
            else if (state->type == StateType::Terminal)
                std::cout << " T  ";
            else
                std::cout << " " << state->policy << "  ";
        }
        std::cout << "\n";
    }

    std::cout << "\nUtilities:\n";
    std::cout << std::fixed << std::setprecision(4);
    for (int y = config.height; y >= 1; --y) {
        for (int x = 1; x <= config.width; ++x) {
            auto state = world.getState(x, y);
            if (state->type == StateType::Forbidden)
                std::cout << " [FORBIDDEN] ";
            else
                std::cout << std::setw(10) << state->utility << " ";
        }
        std::cout << "\n";
    }
}

int main(int argc, char** argv)
{
    std::cout << "Author: " << aiml::author << "\n";

    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <filename> [gamma] [epsilon]\n";
        return 1;
    }

    std::string filename = argv[1];

    // 1. Parse file
    auto world_opt = MDPParser::parseFile(filename);
    if (!world_opt) {
        std::cerr << "Failed to parse MDP configuration from " << filename << "\n";
        return 1;
    }

    GridWorld world = *world_opt;
    MDPConfig config = world.getConfig();

    // Override with command-line arguments if provided
    if (argc >= 3)
        config.gamma = std::stod(argv[2]);
    if (argc >= 4)
        config.epsilon = std::stod(argv[3]);

    // Ensure gamma is set (fallback to 1.0 if missing entirely)
    if (!config.gamma.has_value())
        config.gamma = 1.0;

    world.setConfig(config); // Save updated config

    // 2. Value Iteration
    double delta;
    uint iteration = 0;

    // TODO: Open file for Gnuplot here (e.g. std::ofstream gnuplot_file("convergence.dat");)

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
                    double expected_u = calculate_expected_utility(world, x, y, a);
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

    std::cout << "Value Iteration converged in " << iteration << " iterations.\n";

    // 3. Print the results
    print_world(world);

    return 0;
}