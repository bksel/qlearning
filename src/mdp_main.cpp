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

#include <iostream>

#include "mdp/common_defs.hh"
#include "mdp/mdp_algo.hh"
#include "mdp/mdp_parser.hh"
#include "mdp/visualiser.hh"

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
    mdp::print_policy(world);
    mdp::print_utilities(world);

    return 0;
}