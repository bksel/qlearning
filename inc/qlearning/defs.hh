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

namespace ql {

enum class EAction { Up, Down, Left, Right };

enum class EState { Normal, Start, Terminal, Special, Forbidden };

struct FastState {
    EState type = EState::Normal;
    double reward = 0.0;
};

struct StepResult {
    int next_x;
    int next_y;
    double reward;
    bool is_terminal;
};
} // namespace qlearning