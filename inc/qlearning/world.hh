/**
 * @file world.hh
 * @author Bartosz ksel (275472@student.pwr.edu.pl)
 * @brief Q-learning runner.
 * @version 0.1
 * @date 2026-05-17
 *
 * @copyright Copyright (c) 2026
 *
 */

#include <cstddef>
#include <fstream>
#include <span>
#include <vector>

#include "defs.hh"
#include "fast_rng.hh"
#include "qagent.hh"
#include "runtime_config.hh"
#include "simulator.hh"

namespace ql {

// struct BatchMemory {
//     std::vector<double> Q_table;
//     std::vector<std::size_t> N_table;
// };

struct alignas(std::hardware_destructive_interference_size) BatchMemory {
    std::vector<double> Q_table;
    std::vector<std::size_t> N_table;
};

inline BatchMemory
run_batch(
  const Simulator& sim,
  const BatchConfig& config,
  const BatchMemory& starting_knowledge,
  uint64_t thread_seed
)
{
    BatchMemory mem = starting_knowledge;
    FastRNG rng(thread_seed);
    QAgent agent(mem.Q_table, mem.N_table, config.padded_width, config.gamma, config.epsilon);

    // Standard RL safety cutoff: prevents "straggler" threads from looping infinitely
    const std::size_t MAX_STEPS_PER_EPISODE = 100;

    for (std::size_t t = 0; t < config.trials; ++t) {
        std::size_t cx = 1;
        std::size_t cy = 1;
        std::size_t steps = 0; // Track steps taken in this specific episode

        while (true) {
            EAction action = agent.choose_action(cx, cy, rng);
            StepResult result = sim.execute_action(cx, cy, action, rng);
            agent.learn(cx, cy, action, result);

            steps++;

            // Break if we hit a terminal state OR if the agent is spinning its wheels
            if (result.is_terminal || steps >= MAX_STEPS_PER_EPISODE) {
                break;
            }
            cx = result.next_x;
            cy = result.next_y;
        }
    }
    return mem;
}

// Holds all references needed for the end-of-epoch reduction phase
struct EpochContext {
    ql::BatchMemory& global_batch;
    const std::vector<ql::BatchMemory>& thread_memories;
    const mdp::GridWorld& world;
    const ql::BatchConfig& b_config;
    const ql::RuntimeConfig& run_cfg;
    std::vector<double>& prev_utilities;
    std::ofstream& plot_file;
    std::size_t& current_epoch;
};

// The Callable object passed to std::barrier
struct EpochReducer {
    EpochContext ctx;

    void
    operator()() noexcept
    {
        std::size_t raw_width = ctx.b_config.padded_width - 2;
        std::size_t raw_height = ctx.b_config.padded_height - 2;
        std::size_t trials_per_epoch = ctx.run_cfg.total_trials / ctx.run_cfg.epochs;

        std::cout << "Running epoch (" << ctx.current_epoch << '/' << ctx.run_cfg.epochs << ")...\r"
                  << std::flush;

        std::vector<double> sum_new_Q_mass(ctx.global_batch.Q_table.size(), 0.0);
        std::vector<std::size_t> total_new_visits(ctx.global_batch.N_table.size(), 0);

        // 1. Mathematically Exact Federated Averaging
        for (const auto& local_batch : ctx.thread_memories) {
            for (std::size_t i = 0; i < ctx.global_batch.Q_table.size(); ++i) {
                std::size_t local_N = local_batch.N_table[i];
                std::size_t old_N = ctx.global_batch.N_table[i];

                if (local_N > old_N) {
                    std::size_t delta_N = local_N - old_N;
                    double old_Q = ctx.global_batch.Q_table[i];
                    double thread_target_sum = (local_batch.Q_table[i] * local_N) - (old_Q * old_N);

                    sum_new_Q_mass[i] += thread_target_sum;
                    total_new_visits[i] += delta_N;
                }
            }
        }

        // Apply Global Average
        for (std::size_t i = 0; i < ctx.global_batch.Q_table.size(); ++i) {
            if (total_new_visits[i] > 0) {
                std::size_t old_N = ctx.global_batch.N_table[i];
                double old_Q = ctx.global_batch.Q_table[i];
                std::size_t combined_N = old_N + total_new_visits[i];
                double historical_mass = old_Q * static_cast<double>(old_N);

                ctx.global_batch.Q_table[i] =
                  (historical_mass + sum_new_Q_mass[i]) / static_cast<double>(combined_N);
                ctx.global_batch.N_table[i] = combined_N;
            }
        }

        // 2. Track Convergence for EVERY individual state
        if (ctx.current_epoch == 1) {
            ctx.plot_file << "#Trials";
            for (std::size_t y = 1; y <= raw_height; ++y) {
                for (std::size_t x = 1; x <= raw_width; ++x) {
                    auto state = ctx.world.getState(x, y);
                    if (!state || state->type == mdp::EState::Terminal) continue;
                    ctx.plot_file << "\tD(" << x << "," << y << ")";
                }
            }
            ctx.plot_file << "\n";
        }

        ctx.plot_file << (ctx.current_epoch * trials_per_epoch);
        for (std::size_t y = 1; y <= raw_height; ++y) {
            for (std::size_t x = 1; x <= raw_width; ++x) {
                auto state = ctx.world.getState(x, y);
                if (!state || state->type == mdp::EState::Terminal) continue;

                double max_q = -std::numeric_limits<double>::infinity();
                for (std::size_t a = 0; a < 4; ++a) {
                    std::size_t q_idx = ((y * ctx.b_config.padded_width + x) * 4) + a;
                    if (ctx.global_batch.Q_table[q_idx] > max_q)
                        max_q = ctx.global_batch.Q_table[q_idx];
                }

                std::size_t u_idx = y * ctx.b_config.padded_width + x;
                double delta = std::abs(max_q - ctx.prev_utilities[u_idx]);
                ctx.prev_utilities[u_idx] = max_q;

                ctx.plot_file << "\t" << delta;
            }
        }
        ctx.plot_file << "\n";
        ctx.current_epoch++;
    }
};

} // namespace ql