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
        // Epsilon-Greedy: Explore
        if (rng.next_double() < epsilon) {
            return static_cast<mdp::EAction>(rng.next() % 4);
        }

        // Exploit: Find the action with the maximum Q-value
        double max_q = std::numeric_limits<double>::lowest();
        mdp::EAction best_a = mdp::EAction::Up;

        for (int a = 0; a < 4; ++a) {
            double q_val = Q[q_idx(x, y, static_cast<mdp::EAction>(a))];
            if (q_val > max_q) {
                max_q = q_val;
                best_a = static_cast<mdp::EAction>(a);
            }
        }
        return best_a;
    }

    void learn(int curr_x, int curr_y, EAction action, const StepResult& result) noexcept
    {
        // TODO: Implement Q-Learning update rule
        // alpha = 1.0 / ++N[q_idx(curr_x, curr_y, action)]
        // new_Q = old_Q + alpha * (reward + gamma * max_next_Q - old_Q)

        size_t idx = q_idx(curr_x, curr_y, action);

        // Update visit count and calculate learning rate (alpha = 1 / N(s,a))
        int n = ++N[idx];
        double alpha = 1.0 / static_cast<double>(n);
        double old_q = Q[idx];

        // Find max Q of the NEXT state
        double max_next_q = 0.0;
        if (!result.is_terminal) {
            max_next_q = std::numeric_limits<double>::lowest();
            for (int a = 0; a < 4; ++a) {
                double q_val = Q[q_idx(result.next_x, result.next_y, static_cast<mdp::EAction>(a))];
                if (q_val > max_next_q) max_next_q = q_val;
            }
        }

        // Q-Learning Formula
        Q[idx] = old_q + alpha * (result.reward + gamma * max_next_q - old_q);
    }
};

} // namespace ql