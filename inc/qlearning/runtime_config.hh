/**
 * @file runtime_config.hh
 * @author Bartosz ksel (275472@student.pwr.edu.pl)
 * @brief Q-learning experiment runtime configuration.
 * @version 0.1
 * @date 2026-05-17
 *
 * @copyright Copyright (c) 2026
 *
 */

#pragma once

#include <cstddef>
#include <cstdlib>
#include <iostream>
#include <string>
#include <thread>

namespace ql {

struct RuntimeConfig {
    std::size_t num_threads;
    std::size_t epochs;
    std::size_t total_trials;

    RuntimeConfig()
    {
        // 1. Set Hardware/Logical Defaults
        num_threads = std::thread::hardware_concurrency();
        if (num_threads == 0) num_threads = 4;

        epochs = 100;
        total_trials = 10000000000u;

        // 2. Parse Environment Overrides safely
        num_threads = parse_env("MDP_NUM_THREADS", num_threads);
        epochs = parse_env("MDP_EPOCHS", epochs);
        total_trials = parse_env("MDP_TOTAL_TRIALS", total_trials);
    }

private:
    static int parse_env(const char* env_var, int default_val)
    {
        if (const char* env_str = std::getenv(env_var)) {
            try {
                int parsed = std::stoi(env_str);
                if (parsed > 0) return parsed;
                std::cerr << "Warning: " << env_var << " must be > 0. Using default ("
                          << default_val << ").\n";
            } catch (...) {
                std::cerr << "Warning: Failed to parse " << env_var << " ('" << env_str
                          << "'). Using default (" << default_val << ").\n";
            }
        }
        return default_val;
    }
};
} // namespace ql