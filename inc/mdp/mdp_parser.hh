/**
 * @file mdp_parser.hh
 * @author Bartosz ksel (275472@student.pwr.edu.pl)
 * @brief Provides parsing of configuration files.
 * @version 0.2
 * @date 2026-05-16
 *
 * @copyright Copyright (c) 2026
 *
 */

#pragma once

#include <fstream>
#include <sstream>
#include <string>

#include "core_optional.hh"
#include "grid_world.hh"

namespace mdp {

class MDPParser {
public:
    static core::optional<GridWorld> parseFile(const std::string& filename)
    {
        std::ifstream file(filename);
        if (!file.is_open()) {
            return core::nullopt;
        }

        MDPConfig config;
        std::string line;

        while (std::getline(file, line)) {
            std::istringstream iss(line);
            std::string label;
            if (iss >> label && label == "W") {
                iss >> config.width >> config.height;
                break;
            }
        }

        if (config.width == 0 || config.height == 0) {
            return core::nullopt;
        }

        GridWorld world(config.width, config.height);

        // Reset file to read all other parameters
        file.clear();
        file.seekg(0);

        while (std::getline(file, line)) {
            std::istringstream iss(line);
            std::string label;
            if (!(iss >> label)) continue;
            if (label == "W") continue;

            if (label == "S") {
                int x, y;
                iss >> x >> y;
                config.start_pos = {{x, y}};
                if (auto state = world.getState(x, y)) {
                    state->type = EState::Start;
                }
            } else if (label == "P") {
                iss >> config.p1 >> config.p2 >> config.p3;
            } else if (label == "R") {
                iss >> config.default_reward;
                // Apply default reward to all normal states
                for (int y = 1; y <= config.height; ++y) {
                    for (int x = 1; x <= config.width; ++x) {
                        if (auto state = world.getState(x, y)) {
                            state->reward = config.default_reward;
                        }
                    }
                }
            } else if (label == "G") {
                double g;
                iss >> g;
                config.gamma = g;
            } else if (label == "E") {
                double e;
                iss >> e;
                config.epsilon = e;
            } else if (label == "T") {
                int x, y;
                double r;
                iss >> x >> y >> r;
                if (auto state = world.getState(x, y)) {
                    state->type = EState::Terminal;
                    state->reward = r;
                    state->utility = r;
                }
            } else if (label == "B") {
                int x, y;
                double r;
                iss >> x >> y >> r;
                if (auto state = world.getState(x, y)) {
                    state->type = EState::Special;
                    state->reward = r;
                }
            } else if (label == "F") {
                int x, y;
                iss >> x >> y;
                if (auto state = world.getState(x, y)) {
                    state->type = EState::Forbidden;
                }
            }
        }

        world.setConfig(config);
        return world;
    }
};

} // namespace mdp