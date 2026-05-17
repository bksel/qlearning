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

#include <iomanip>
#include <iostream>

#include "mdp/common_defs.hh"
#include "mdp/mdp_algo.hh"
#include "mdp/mdp_parser.hh"

// Helper to print the optimal policy in a grid
void print_policy(const mdp::GridWorld& world)
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
            if (state->type == mdp::StateType::Forbidden)
                std::cout << " F |";
            else if (state->type == mdp::StateType::Terminal)
                std::cout << " T |";
            else
                std::cout << " " << state->policy << " |";
        }
        std::cout << "\n";
    }
    std::cout << hline << "\n";
}

void print_utilities(const mdp::GridWorld& world)
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
            if (state->type == mdp::StateType::Forbidden)
                std::cout << "    F     |";
            else
                std::cout << " " << std::setw(8) << state->utility << " |";
        }
        std::cout << "\n";
    }
    std::cout << hline << "\n";
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
    auto world_opt = mdp::MDPParser::parseFile(filename);
    if (!world_opt) {
        std::cerr << "Failed to parse MDP configuration from " << filename << "\n";
        return 1;
    }

    mdp::GridWorld world = *world_opt;
    mdp::MDPConfig config = world.getConfig();

    // Override with command-line arguments if provided
    if (argc >= 3) {
        config.gamma = std::stod(argv[2]);
    }
    if (argc >= 4) {
        config.epsilon = std::stod(argv[3]);
    }
    // Ensure gamma is set (fallback to 1.0 if missing entirely)
    if (!config.gamma.has_value()) {
        config.gamma = 1.0;
    }
    world.setConfig(config); // Save updated config

    std::ofstream gnuplot_file("convergence.dat");
    if (!gnuplot_file.is_open()) {
        std::cerr << "Warning: Could not open convergence.dat for writing.\n";
    }

    uint iteration =
        mdp::calculate_policies(world, config, gnuplot_file.is_open() ? &gnuplot_file : nullptr);

    std::cout << "Value Iteration converged in " << iteration << " iterations.\n";

    // 3. Print the results
    print_policy(world);
    print_utilities(world);

    return 0;
}