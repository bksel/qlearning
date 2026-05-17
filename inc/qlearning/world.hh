#include <future>
#include <iostream>
#include <limits>
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

inline BatchMemory
run_batch(const Simulator& sim, int p_width, int p_height, int trials, double gamma, double epsilon)
{
    BatchMemory mem;
    mem.Q_table.assign(p_width * p_height * 4, 0.0);
    mem.N_table.assign(p_width * p_height * 4, 0);

    // Give this thread a unique seed!
    FastRNG rng(std::random_device{}());
    QAgent agent(mem.Q_table, mem.N_table, p_width, gamma, epsilon);

    for (int t = 0; t < trials; ++t) {
        int cx = 1; // TODO: Set to starting X
        int cy = 1; // TODO: Set to starting Y

        while (true) {
            EAction action = agent.choose_action(cx, cy, rng);
            StepResult result = sim.execute_action(cx, cy, action, rng);
            agent.learn(cx, cy, action, result);

            if (result.is_terminal) break;
            cx = result.next_x;
            cy = result.next_y;
        }
    }
    return mem;
}

} // namespace qlearning