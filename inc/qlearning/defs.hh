/**
 * @file defs.hh
 * @author Bartosz ksel (275472@student.pwr.edu.pl)
 * @brief Data structures used in Q-learning.
 * @version 0.1
 * @date 2026-05-16
 *
 * @copyright Copyright (c) 2026
 *
 */

#pragma once

#include "mdp/common_defs.hh"
#include "mdp/grid_world.hh"

namespace ql {

using EAction = mdp::EAction;

using EState = mdp::EState;

struct FastState {
    EState type = EState::Normal;
    double reward = 0.0;
};

struct StepResult {
    std::size_t next_x = 0;
    std::size_t next_y = 0;
    double reward = 0.0;
    bool is_terminal = false;
};

// Clean config encapsulation structure
struct BatchConfig {
    std::size_t padded_width = 0;
    std::size_t padded_height = 0;
    std::size_t trials = 0;
    double gamma = 1.0;
    double epsilon = 0.05;
};
} // namespace ql