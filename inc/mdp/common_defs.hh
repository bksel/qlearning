/**
 * @file common_defs.hh
 * @author Bartosz ksel (275472@student.pwr.edu.pl)
 * @brief Common project definitions.
 * @version 0.1
 * @date 2026-05-16
 *
 * @copyright Copyright (c) 2026
 *
 */

#pragma once
#include <string_view>

namespace aiml {

static constexpr std::string_view author = "Bartosz Ksel (275472)";

}

namespace mdp {

enum class EAction { Up, Down, Left, Right };

// Convert action to required character representation
inline char action_to_char(EAction a)
{
    switch (a) {
    case EAction::Up: return '^';
    case EAction::Down: return 'v';
    case EAction::Left: return '<';
    case EAction::Right: return '>';
    }
    return '*';
}

// Helper to get movement deltas
inline std::pair<int, int> get_dir(EAction a)
{
    switch (a) {
    case EAction::Up: return {0, 1};
    case EAction::Down: return {0, -1};
    case EAction::Left: return {-1, 0};
    case EAction::Right: return {1, 0};
    }
    return {0, 0};
}

// Relative direction mappings based on intended action
inline EAction get_left_action(EAction a)
{
    switch (a) {
    case EAction::Up: return EAction::Left;
    case EAction::Down: return EAction::Right;
    case EAction::Left: return EAction::Down;
    case EAction::Right: return EAction::Up;
    }
    return EAction::Up;
}

inline EAction get_right_action(EAction a)
{
    switch (a) {
    case EAction::Up: return EAction::Right;
    case EAction::Down: return EAction::Left;
    case EAction::Left: return EAction::Up;
    case EAction::Right: return EAction::Down;
    }
    return EAction::Up;
}

inline EAction get_opposite_action(EAction a)
{
    switch (a) {
    case EAction::Up: return EAction::Down;
    case EAction::Down: return EAction::Up;
    case EAction::Left: return EAction::Right;
    case EAction::Right: return EAction::Left;
    }
    return EAction::Up;
}
} // namespace mdp