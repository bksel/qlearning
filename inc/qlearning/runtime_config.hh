/**
 * @file runtime_config.hh
 * @author Bartosz ksel (275472@student.pwr.edu.pl)
 * @brief Environment configuration parser for Q-learning.
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
        // 1. Set Hardware Defaults using size_t
        num_threads = static_cast<std::size_t>(std::thread::hardware_concurrency());
        if (num_threads == 0) num_threads = 4;

        epochs = 100;
        total_trials = 1000000; // 1 Million default

        // 2. Parse Environment Overrides safely using 64-bit unsigned loader
        num_threads = parse_env_size("MDP_NUM_THREADS", num_threads);
        epochs = parse_env_size("MDP_EPOCHS", epochs);
        total_trials = parse_env_size("MDP_TOTAL_TRIALS", total_trials);
    }

private:
    static std::size_t parse_env_size(const char* env_var, std::size_t default_val)
    {
        if (const char* env_str = std::getenv(env_var)) {
            try {
                // stoull parses directly up to 18 Quintillion
                unsigned long long parsed = std::stoull(env_str);
                if (parsed > 0) {
                    return static_cast<std::size_t>(parsed);
                }
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