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
#include <random>
#include <span>
#include <vector>

#include "defs.hh"
#include "fast_rng.hh"
#include "qagent.hh"
#include "simulator.hh"

namespace ql {

struct BatchMemory {
    std::vector<double> Q_table;
    std::vector<int> N_table;
};

inline BatchMemory run_batch(const Simulator& sim,
                             const BatchConfig& config,
                             const BatchMemory& starting_knowledge,
                             uint64_t thread_seed)
{
    // Initialize thread-local memory with the GLOBAL knowledge!
    BatchMemory mem = starting_knowledge;

    // Use deterministic seed passed from main
    FastRNG rng(thread_seed);
    QAgent agent(mem.Q_table, mem.N_table, config.padded_width, config.gamma, config.epsilon);

    for (std::size_t t = 0; t < config.trials; ++t) {
        std::size_t cx = 1;
        std::size_t cy = 1;

        while (true) {
            EAction action = agent.choose_action(cx, cy, rng);
            StepResult result = sim.execute_action(cx, cy, action, rng);
            agent.learn(cx, cy, action, result);

            if (result.is_terminal) {
                break;
            }
            cx = result.next_x;
            cy = result.next_y;
        }
    }
    return mem;
}

} // namespace ql