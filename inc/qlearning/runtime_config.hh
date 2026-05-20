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

    static RuntimeConfig
    from_enviromental_variables()
    {
        RuntimeConfig rc;
        constexpr std::size_t MIN_TRIALS_PER_THREAD = 100'000;

        std::size_t max_hardware = static_cast<std::size_t>(std::thread::hardware_concurrency());
        if (max_hardware == 0) max_hardware = 4;

        // 1. Parse Environment Overrides (as you already do)
        std::size_t requested_threads = parse_env_size("MDP_NUM_THREADS", max_hardware);
        rc.epochs = parse_env_size("MDP_EPOCHS", 100);
        rc.total_trials = parse_env_size("MDP_TOTAL_TRIALS", 1000000);

        // 2. Calculate the maximum number of threads that actually make sense
        std::size_t trials_per_epoch = rc.total_trials / rc.epochs;
        std::size_t max_useful_threads =
          std::max<std::size_t>(1, trials_per_epoch / MIN_TRIALS_PER_THREAD);

        // 3. Clamp the final thread count
        rc.num_threads = std::min(requested_threads, max_useful_threads);

        return rc;
    }

private:
    static std::size_t
    parse_env_size(
      const char* env_var,
      std::size_t default_val
    )
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