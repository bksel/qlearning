/**
 * @file grid_world.hh
 * @author Bartosz ksel (275472@student.pwr.edu.pl)
 * @brief  Definitions of grid world.
 * @version 0.2
 * @date 2026-05-16
 *
 * @copyright Copyright (c) 2026
 *
 */

#pragma once

#include <utility>
#include <vector>

#include "core_optional.hh"

// Represents the different types of states in the grid
enum class StateType { Normal, Start, Terminal, Special, Forbidden };

// Represents a single cell/state in the MDP world
struct State {
    StateType type = StateType::Normal;
    double reward = 0.0;  // R(s)
    double utility = 0.0; // U(s) for Part I, or max_a Q(s,a) for Part II
    char policy = '*';    // >, <, ^, v, or * for uninitialized/terminal
};

// Holds all parameters parsed from the configuration file or command line
struct MDPConfig {
    int width = 0;
    int height = 0;
    double p1 = 0.0, p2 = 0.0, p3 = 0.0;
    double default_reward = 0.0;
    core::optional<double> gamma;
    core::optional<double> epsilon;
    core::optional<std::pair<int, int>> start_pos;
};

class GridWorld {
private:
    int width;
    int height;
    MDPConfig config;
    std::vector<State> grid; // Flattened 1D vector for optimal cache locality

    // Converts 1-based bottom-left coordinates to 1D vector index
    core::optional<std::size_t> toInternalIndex(int x, int y) const
    {
        if (x < 1 || x > width || y < 1 || y > height) {
            return core::nullopt;
        }
        return static_cast<size_t>((height - y) * width + (x - 1));
    }

public:
    GridWorld(int w, int h)
      : width(w)
      , height(h)
      , grid(w * h)
    {
    }

    core::optional<State&> getState(int x, int y)
    {
        if (auto idx = toInternalIndex(x, y)) {
            return grid[*idx];
        }
        return core::nullopt;
    }

    core::optional<const State&> getState(int x, int y) const
    {
        if (auto idx = toInternalIndex(x, y)) {
            return grid[*idx];
        }
        return core::nullopt;
    }

    int getWidth() const
    {
        return width;
    }
    int getHeight() const
    {
        return height;
    }
    MDPConfig getConfig() const
    {
        return config;
    }
    void setConfig(const MDPConfig& cfg)
    {
        config = cfg;
    }
};