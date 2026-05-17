/**
 * @file qlearning_main.cpp
 * @author Bartosz ksel (275472@student.pwr.edu.pl)
 * @brief Entrypoint for program done for 2nd task (Q learning).
 * @version 0.1
 * @date 2026-05-16
 *
 * @copyright Copyright (c) 2026
 *
 */

#include <future>
#include <iostream>
#include <limits>
#include <string>
#include <vector>

#include "core_optional.hh"
#include "mdp/common_defs.hh" // Need aiml::author and action_to_char/EAction
#include "mdp/grid_world.hh"
#include "mdp/mdp_parser.hh"
#include "mdp/visualiser.hh"

#include "qlearning/simulator.hh"
#include "qlearning/world.hh"

// You can copy the print_policy and print_utilities functions here from mdp_main.cpp
// void print_policy(const mdp::GridWorld& world) { ... }
// void print_utilities(const mdp::GridWorld& world) { ... }

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
    if (argc >= 3) config.gamma = std::stod(argv[2]);
    if (argc >= 4) config.epsilon = std::stod(argv[3]);

    // Fallbacks
    if (!config.gamma.has_value()) config.gamma = 1.0;
    if (!config.epsilon.has_value()) config.epsilon = 0.05; // Given in assignment

    int raw_width = config.width;
    int raw_height = config.height;
    int padded_width = raw_width + 2;
    int padded_height = raw_height + 2;

    core::optional<ql::Simulator> opt_sim = ql::Simulator::from_gridworld(world);

    if (!opt_sim.has_value()) {
        std::cerr << "Failed to create Q-learning simulation from " << filename << "\n";
        return 1;
    }
    ql::Simulator sim = *opt_sim;

    // 2. Launch Threads
    std::size_t total_trials = 10000 * 1000 * 100; // Assignment says start with 10k, but let's go for precision!
    int num_threads = std::thread::hardware_concurrency();
    if (num_threads == 0) num_threads = 4;

    // temporary:
    // num_threads = 1;

    std::cout << "Starting " << total_trials << " trials across " << num_threads << " threads...\n";

    std::vector<std::future<ql::BatchMemory>> futures;

    for (int i = 0; i < num_threads; ++i) {
        futures.push_back(std::async(std::launch::async, ql::run_batch, std::ref(sim), padded_width,
                                     padded_height, total_trials / num_threads,
                                     config.gamma.value(), config.epsilon.value(), config));
    }

    // 3. Merge Results (Reduce)
    ql::BatchMemory global_batch;
    global_batch.Q_table.assign(padded_width * padded_height * 4, 0.0);
    global_batch.N_table.assign(padded_width * padded_height * 4, 0);

    for (auto& f : futures) {
        ql::BatchMemory local_batch = f.get(); // Blocks until thread completes

        for (size_t i = 0; i < global_batch.Q_table.size(); ++i) {
            int local_N = local_batch.N_table[i];
            if (local_N == 0) continue; // Skip unvisited pairs

            double local_Q = local_batch.Q_table[i];

            // Weighted average based on visit counts
            int old_global_N = global_batch.N_table[i];
            int new_global_N = old_global_N + local_N;

            double current_sum = global_batch.Q_table[i] * old_global_N;
            double local_sum = local_Q * local_N;

            global_batch.Q_table[i] = (current_sum + local_sum) / new_global_N;
            global_batch.N_table[i] = new_global_N;
        }
    }

    std::cout << "High-Speed Q-Learning completed!\n";

    // 4. Extract Policy & Utilities from Global Q-Table
    for (int y = 1; y <= raw_height; ++y) {
        for (int x = 1; x <= raw_width; ++x) {
            auto state = world.getState(x, y);
            if (!state || state->type == mdp::EState::Forbidden ||
                state->type == mdp::EState::Terminal)
                continue;

            double max_q = std::numeric_limits<double>::lowest();
            auto best_action = mdp::EAction::Up;

            // Iterate over the 4 actions to find the best one for this state
            for (int a = 0; a < 4; ++a) {
                // Calculate 1D index in our padded grid setup
                size_t q_idx = ((y * padded_width + x) * 4) + a;
                double q_val = global_batch.Q_table[q_idx];

                if (q_val > max_q) {
                    max_q = q_val;
                    best_action = static_cast<mdp::EAction>(a);
                }
            }

            // Assign U(s) = max_a Q(s, a)
            state->utility = max_q;
            state->policy = action_to_char(best_action);
        }
    }

    // Print out the results identically to part 1!
    mdp::print_utilities(world);
    mdp::print_policy(world);

    return 0;
}