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

#include <cmath>
#include <cstddef>
#include <fstream>
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

#include "qlearning/runtime_config.hh"
#include "qlearning/simulator.hh"
#include "qlearning/world.hh"

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
    if (!config.epsilon.has_value()) config.epsilon = 0.05;

    std::size_t raw_width = config.width;
    std::size_t raw_height = config.height;

    core::optional<ql::Simulator> opt_sim = ql::Simulator::from_gridworld(world);

    if (!opt_sim.has_value()) {
        std::cerr << "Failed to create Q-learning simulation from " << filename << "\n";
        return 1;
    }
    ql::Simulator sim = *opt_sim;

    // 2. Setup Configuration
    ql::RuntimeConfig run_cfg;

    std::size_t trials_per_epoch = run_cfg.total_trials / run_cfg.epochs;
    std::size_t trials_per_thread = trials_per_epoch / run_cfg.num_threads;

    ql::BatchConfig b_config;
    b_config.padded_width = raw_width + 2;
    b_config.padded_height = raw_height + 2;
    b_config.trials = trials_per_thread; // Pass the divided chunk!
    b_config.gamma = config.gamma.value();
    b_config.epsilon = config.epsilon.value();

    std::cout << "Starting Q-Learning Experiment:\n"
              << "  Threads: " << run_cfg.num_threads << "\n"
              << "  Epochs: " << run_cfg.epochs << "\n"
              << "  Total Trials: " << run_cfg.total_trials << "\n"
              << "  Trials per Epoch: " << trials_per_epoch << "\n\n";

    // 3. Initialize Global Memory & Logging
    ql::BatchMemory global_batch;
    global_batch.Q_table.assign(b_config.padded_width * b_config.padded_height * 4, 0.0);
    global_batch.N_table.assign(b_config.padded_width * b_config.padded_height * 4, 0);

    std::vector<double> prev_utilities(b_config.padded_width * b_config.padded_height, 0.0);
    std::ofstream plot_file("qlearning_convergence.dat");
    plot_file << "# Trials\tMax_Delta\n";

    uint64_t master_seed = 1337;

    // 4. The Epoch Training Loop
    for (int epoch = 1; epoch <= run_cfg.epochs; ++epoch) {
        std::cout << "Running epoch (" << epoch << '/' << run_cfg.epochs << ")\n";
        std::vector<std::future<ql::BatchMemory>> futures;

        // Launch Threads
        for (int i = 0; i < run_cfg.num_threads; ++i) {
            uint64_t thread_seed = master_seed + (epoch * 1000) + i;
            futures.push_back(std::async(std::launch::async, ql::run_batch, std::ref(sim), b_config,
                                         global_batch, thread_seed));
        }

        // Merge Results (Reduce)
        ql::BatchMemory next_global = global_batch;
        // Reset N_table so we only weight the NEW visits from this epoch!
        next_global.N_table.assign(b_config.padded_width * b_config.padded_height * 4, 0);

        std::vector<double> sum_delta_Q(b_config.padded_width * b_config.padded_height * 4, 0.0);
        std::vector<int> sum_delta_N(b_config.padded_width * b_config.padded_height * 4, 0);
        std::vector<int> active_threads(b_config.padded_width * b_config.padded_height * 4, 0);

        for (auto& f : futures) {
            ql::BatchMemory local_batch = f.get();

            for (std::size_t i = 0; i < global_batch.Q_table.size(); ++i) {
                // How many NEW visits did this specific thread make?
                int delta_N = local_batch.N_table[i] - global_batch.N_table[i];

                if (delta_N > 0) {
                    sum_delta_N[i] += delta_N;
                    sum_delta_Q[i] += (local_batch.Q_table[i] - global_batch.Q_table[i]);
                    active_threads[i]++;
                }
            }
        }

        // Apply the averaged updates back to the master global batch
        for (std::size_t i = 0; i < global_batch.Q_table.size(); ++i) {
            if (active_threads[i] > 0) {
                global_batch.Q_table[i] += (sum_delta_Q[i] / active_threads[i]);
                global_batch.N_table[i] += sum_delta_N[i];
            }
        }


        // Calculate Convergence (Delta)
        double max_delta = 0.0;
        for (std::size_t y = 1; y <= raw_height; ++y) {
            for (std::size_t x = 1; x <= raw_width; ++x) {
                auto state = world.getState(x, y);
                if (!state || state->type == mdp::EState::Forbidden ||
                    state->type == mdp::EState::Terminal)
                    continue;

                double max_q = -std::numeric_limits<double>::infinity();
                for (std::size_t a = 0; a < 4; ++a) {
                    std::size_t q_idx = ((y * b_config.padded_width + x) * 4) + a;
                    if (global_batch.Q_table[q_idx] > max_q) max_q = global_batch.Q_table[q_idx];
                }

                std::size_t u_idx = y * b_config.padded_width + x;
                double delta = std::abs(max_q - prev_utilities[u_idx]);
                if (delta > max_delta) max_delta = delta;

                prev_utilities[u_idx] = max_q; // Save for next epoch
            }
        }

        // Log to file
        plot_file << (epoch * trials_per_epoch) << "\t" << max_delta << "\n";
    }

    std::cout << "High-Speed Q-Learning completed! Data written to qlearning_convergence.dat\n";

    // 5. Extract Final Policy & Utilities
    for (std::size_t y = 1; y <= raw_height; ++y) {
        for (std::size_t x = 1; x <= raw_width; ++x) {
            auto state = world.getState(x, y);
            if (!state || state->type == mdp::EState::Forbidden ||
                state->type == mdp::EState::Terminal)
                continue;

            double max_q = -std::numeric_limits<double>::infinity();
            auto best_action = mdp::EAction::Up;

            for (std::size_t a = 0; a < 4; ++a) {
                std::size_t q_idx = ((y * b_config.padded_width + x) * 4) + a;
                double q_val = global_batch.Q_table[q_idx];

                if (q_val > max_q) {
                    max_q = q_val;
                    best_action = static_cast<mdp::EAction>(a);
                }
            }

            state->utility = max_q;
            state->policy = action_to_char(best_action);
        }
    }

    mdp::print_utilities(world);
    mdp::print_policy(world);

    return 0;
}