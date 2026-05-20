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

#include <barrier>
#include <cmath>
#include <cstddef>
#include <fstream>
#include <future>
#include <iostream>
#include <limits>
#include <string>
#include <thread>
#include <vector>

#include "core_optional.hh"
#include "mdp/common_defs.hh" // Need aiml::author and action_to_char/EAction
#include "mdp/grid_world.hh"
#include "mdp/mdp_parser.hh"
#include "mdp/visualiser.hh"

#include "qlearning/runtime_config.hh"
#include "qlearning/simulator.hh"
#include "qlearning/world.hh"

void
print_q_values(
  const mdp::GridWorld& world,
  const ql::BatchMemory& global_batch,
  const ql::BatchConfig& b_config
)
{
    std::cout << "\nQ-Values (1 decimal place):\n";

    std::size_t raw_width = b_config.padded_width - 2;
    std::size_t raw_height = b_config.padded_height - 2;

    // Create the horizontal border (15 characters per cell)
    std::string hline = "+";
    for (std::size_t i = 0; i < raw_width; ++i) hline += "---------------+";

    std::cout << std::fixed << std::setprecision(1);

    for (std::size_t y = raw_height; y >= 1; --y) {
        std::cout << hline << "\n|";

        // --- TOP ROW: Up Actions ---
        for (std::size_t x = 1; x <= raw_width; ++x) {
            auto state = world.getState(x, y);
            if (state->type == mdp::EState::Forbidden || state->type == mdp::EState::Terminal) {
                std::cout << "               |";
            } else {
                std::size_t q_idx = ((y * b_config.padded_width + x) * 4) +
                                    static_cast<std::size_t>(mdp::EAction::Up);
                std::cout << "    ^ " << std::setw(4) << global_batch.Q_table[q_idx] << "     |";
            }
        }
        std::cout << "\n|";

        // --- MIDDLE ROW: Left & Right Actions (and F/T Labels) ---
        for (std::size_t x = 1; x <= raw_width; ++x) {
            auto state = world.getState(x, y);
            if (state->type == mdp::EState::Forbidden) {
                std::cout << "       F       |";
            } else if (state->type == mdp::EState::Terminal) {
                std::cout << "     T " << std::setw(4) << state->reward << "    |";
            } else {
                std::size_t q_l = ((y * b_config.padded_width + x) * 4) +
                                  static_cast<std::size_t>(mdp::EAction::Left);
                std::size_t q_r = ((y * b_config.padded_width + x) * 4) +
                                  static_cast<std::size_t>(mdp::EAction::Right);
                std::cout << " <" << std::setw(4) << global_batch.Q_table[q_l] << "   >"
                          << std::setw(4) << global_batch.Q_table[q_r] << " |";
            }
        }
        std::cout << "\n|";

        // --- BOTTOM ROW: Down Actions ---
        for (std::size_t x = 1; x <= raw_width; ++x) {
            auto state = world.getState(x, y);
            if (state->type == mdp::EState::Forbidden || state->type == mdp::EState::Terminal) {
                std::cout << "               |";
            } else {
                std::size_t q_idx = ((y * b_config.padded_width + x) * 4) +
                                    static_cast<std::size_t>(mdp::EAction::Down);
                std::cout << "    v " << std::setw(4) << global_batch.Q_table[q_idx] << "     |";
            }
        }
        std::cout << "\n";
    }
    std::cout << hline << "\n";
}

inline uint64_t
mix_seed(uint64_t z) noexcept
{
    z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9ULL;
    z = (z ^ (z >> 27)) * 0x94d049bb133111ebULL;
    return z ^ (z >> 31);
}

ql::BatchMemory
run_training_epochs(
  ql::Simulator& sim,
  mdp::GridWorld& world,
  const ql::RuntimeConfig& run_cfg,
  const ql::BatchConfig& b_config
)
{
    ql::BatchMemory global_batch;
    global_batch.Q_table.assign(b_config.padded_width * b_config.padded_height * 4, 0.0);
    global_batch.N_table.assign(b_config.padded_width * b_config.padded_height * 4, 0);

    std::vector<ql::BatchMemory> thread_memories(run_cfg.num_threads, global_batch);
    std::vector<double> prev_utilities(b_config.padded_width * b_config.padded_height, 0.0);
    std::ofstream plot_file("qlearning_convergence.dat");
    std::size_t current_epoch = 1;

    // Pack the context and instantiate the barrier
    ql::EpochContext ctx{
      global_batch,
      thread_memories,
      world,
      b_config,
      run_cfg,
      prev_utilities,
      plot_file,
      current_epoch
    };
    std::barrier sync_point(run_cfg.num_threads, ql::EpochReducer{ctx});

    uint64_t master_seed = 1337;
    std::vector<std::jthread> threads;

    for (std::size_t i = 0; i < run_cfg.num_threads; ++i) {
        threads.emplace_back([&, i]() {
            uint64_t thread_seed = mix_seed(master_seed + i * 1000);

            for (std::size_t epoch = 1; epoch <= run_cfg.epochs; ++epoch) {
                thread_memories[i] =
                  ql::run_batch(sim, b_config, global_batch, thread_seed + epoch);
                sync_point.arrive_and_wait();
            }
        });
    }

    // jthreads automatically join on destruction, blocking here until finished
    threads.clear();

    std::cout << "\nHigh-Speed Q-Learning completed! Data written to qlearning_convergence.dat\n";
    return global_batch;
}

void
apply_qtable_to_world(
  mdp::GridWorld& world,
  const ql::BatchMemory& global_batch,
  const ql::BatchConfig& b_config
)
{
    std::size_t raw_width = b_config.padded_width - 2;
    std::size_t raw_height = b_config.padded_height - 2;

    for (std::size_t y = 1; y <= raw_height; ++y) {
        for (std::size_t x = 1; x <= raw_width; ++x) {
            auto state = world.getState(x, y);
            if (
              !state || state->type == mdp::EState::Forbidden ||
              state->type == mdp::EState::Terminal
            )
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

            // state->utility = max_q;
            state->utility = state->reward + b_config.gamma * max_q;
            state->policy = mdp::action_to_char(best_action);
        }
    }
}

int
main(
  int argc,
  char** argv
)
{
    std::cout << "Author: " << aiml::author << "\n";

    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <filename> [gamma] [epsilon]\n";
        return 1;
    }

    // 1. Parse File & Configuration
    auto world_opt = mdp::MDPParser::parseFile(argv[1]);
    if (!world_opt) return 1;
    mdp::GridWorld world = *world_opt;
    mdp::MDPConfig config = world.getConfig();

    config.gamma = (argc >= 3) ? std::stod(argv[2]) : config.gamma.value_or(1.0);
    config.epsilon = (argc >= 4) ? std::stod(argv[3]) : config.epsilon.value_or(0.05);

    auto sim_opt = ql::Simulator::from_gridworld(world);
    if (!sim_opt) return 1;
    ql::Simulator sim = *sim_opt;

    // 2. Setup Runtime Environment
    auto run_cfg = ql::RuntimeConfig::from_enviromental_variables();
    ql::BatchConfig b_config;
    b_config.padded_width = config.width + 2;
    b_config.padded_height = config.height + 2;
    b_config.trials = (run_cfg.total_trials / run_cfg.epochs) / run_cfg.num_threads;
    b_config.gamma = config.gamma.value();
    b_config.epsilon = config.epsilon.value();

    std::cout << "Starting Q-Learning Experiment:\n"
              << "  Threads: " << run_cfg.num_threads << "\n"
              << "  Epochs: " << run_cfg.epochs << "\n"
              << "  Total Trials: " << run_cfg.total_trials << "\n\n";

    // 3. Execute Experiment
    ql::BatchMemory final_memory = run_training_epochs(sim, world, run_cfg, b_config);

    // 4. Extract and Visualise Results
    apply_qtable_to_world(world, final_memory, b_config);
    mdp::print_utilities<mdp::EForbiddenState::DoNotShow>(world);
    mdp::print_policy<mdp::EForbiddenState::DoNotShow>(world);
    print_q_values(world, final_memory, b_config); // <--- Add this here

    return 0;
}