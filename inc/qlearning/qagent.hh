/**
 * @file qagenr.hh
 * @author Bartosz ksel (275472@student.pwr.edu.pl)
 * @brief Agent learning using Q-learning algortihm.
 * @version 0.1
 * @date 2026-05-16
 *
 * @copyright Copyright (c) 2026
 *
 */

#include <span>

#include "defs.hh"
#include "fast_rng.hh"

namespace ql {

class QAgent {
private:
    std::span<double> Q;
    std::span<int> N;
    int padded_width;
    double gamma, epsilon;

    inline size_t q_idx(int x, int y, EAction a) const noexcept
    {
        return ((y * padded_width + x) * 4) + static_cast<int>(a);
    }

public:
    QAgent(std::span<double> q, std::span<int> n, int w, double g, double e)
      : Q(q)
      , N(n)
      , padded_width(w)
      , gamma(g)
      , epsilon(e)
    {
    }

    EAction choose_action(int x, int y, FastRNG& rng) const noexcept
    {
        // TODO: Implement Epsilon-Greedy logic using Q-table and rng.next_double()
        return static_cast<EAction>(rng.next() % 4);
    }

    void learn(int curr_x, int curr_y, EAction action, const StepResult& result) noexcept
    {
        // TODO: Implement Q-Learning update rule
        // alpha = 1.0 / ++N[q_idx(curr_x, curr_y, action)]
        // new_Q = old_Q + alpha * (reward + gamma * max_next_Q - old_Q)
    }
};

} // namespace qlearning