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
    std::size_t padded_width;
    double gamma, epsilon;

    inline std::size_t q_idx(std::size_t x, std::size_t y, EAction a) const noexcept
    {
        return ((y * padded_width + x) * 4) + static_cast<std::size_t>(a);
    }

public:
    QAgent(std::span<double> q, std::span<int> n, std::size_t w, double g, double e)
      : Q(q)
      , N(n)
      , padded_width(w)
      , gamma(g)
      , epsilon(e)
    {
    }

    EAction choose_action(std::size_t x, std::size_t y, FastRNG& rng) const noexcept
    {
        // Epsilon-Greedy selection
        if (rng.next_double() < epsilon) {
            return static_cast<EAction>(rng.next() % 4); // Random exploration
        }

        // Exploitation: search for the highest evaluated Q-value
        double max_q = -std::numeric_limits<double>::infinity();
        EAction best_action = EAction::Up;

        for (std::size_t a = 0; a < 4; ++a) {
            double q_val = Q[q_idx(x, y, static_cast<EAction>(a))];
            if (q_val > max_q) {
                max_q = q_val;
                best_action = static_cast<EAction>(a);
            }
        }
        return best_action;
    }

    void learn(std::size_t curr_x,
               std::size_t curr_y,
               EAction action,
               const StepResult& result) noexcept
    {
        std::size_t idx = q_idx(curr_x, curr_y, action);

        // Tabular update tracking
        int visit_count = ++N[idx];
        double alpha = 1.0 / static_cast<double>(visit_count);

        // Compute max Q(s', a') for the following state if non-terminal
        double max_next_q = 0.0;
        if (!result.is_terminal) {
            max_next_q = -std::numeric_limits<double>::infinity();
            for (std::size_t a = 0; a < 4; ++a) {
                double q_val = Q[q_idx(result.next_x, result.next_y, static_cast<EAction>(a))];
                if (q_val > max_next_q) {
                    max_next_q = q_val;
                }
            }
        }

        // Bellman temporal difference backup
        double old_q = Q[idx];
        Q[idx] = old_q + alpha * (result.reward + gamma * max_next_q - old_q);
    }
};

} // namespace ql