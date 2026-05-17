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
                             std::size_t p_width,
                             std::size_t p_height,
                             std::size_t trials,
                             double gamma,
                             double epsilon,
                             mdp::MDPConfig config)
{
    BatchMemory mem;
    mem.Q_table.assign(p_width * p_height * 4, 0.0);
    mem.N_table.assign(p_width * p_height * 4, 0);

    FastRNG rng(std::random_device{}());
    QAgent agent(mem.Q_table, mem.N_table, p_width, gamma, epsilon);

    for (std::size_t t = 0; t < trials; ++t) {
        int cx, cy;

        // Start state logic based on assignment requirements
        if (config.start_pos.has_value()) {
            cx = config.start_pos->first;
            cy = config.start_pos->second;
        } else {
            // TODO (Optional but required if start missing): Pick a random valid x, y
            // For now, defaulting to 1, 1 to keep it compiling
            cx = 1;
            cy = 1;
        }

        while (true) {
            mdp::EAction action = agent.choose_action(cx, cy, rng);
            StepResult result = sim.execute_action(cx, cy, action, rng);
            agent.learn(cx, cy, action, result);

            if (result.is_terminal) break;

            cx = result.next_x;
            cy = result.next_y;
        }
    }
    return mem;
}

} // namespace ql