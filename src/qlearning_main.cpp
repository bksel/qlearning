/**
 * @file qlearning_main.cpp
 * @author Bartosz ksel (275472@student.pwr.edu.pl)
 * @brief Entrypoint for program done for 2nd task (Q learning).
 * @version 0.1
 * @date 2026-05-16
 *
 * @copyright Copyright (c) 2026
 *
 */

#include "qlearning/world.hh"



int main()
{
    // 1. Parse File & Setup
    int raw_width = 4, raw_height = 3;
    ql::Simulator sim(raw_width, raw_height, 0.8, 0.1, 0.1);

    // 2. Launch Threads
    int total_trials = 100000;
    int num_threads = 4;
    std::vector<std::future<ql::BatchMemory>> futures;

    for (int i = 0; i < num_threads; ++i) {
        futures.push_back(std::async(std::launch::async, ql::run_batch, std::ref(sim), raw_width + 2,
                                     raw_height + 2, total_trials / num_threads, 0.99, 0.1));
    }

    // 3. Merge Results (Reduce)
    // TODO: Create a global Q/N table and do the weighted average as discussed

    std::cout << "High-Speed Q-Learning completed!\n";
    return 0;
}