# AIML Project 4 -- Markov's Decision Problem

This project implements solvers for discrete Markov Decision Problems (MDP) using direct methods (Value Iteration) and Reinforcement Learning (Q-Learning).

## Building the Project

Ninja is used by default as the build generator, but you can change it to Make by specifying `GENERATOR=make`.

```
make build
# Or to explicitly use Make instead of Ninja:
make build GENERATOR=make
```

_NOTE_:
There is a warning during compilation: `project4/inc/qlearning/world.hh:30:21: warning: use of ‘std::hardware_destructive_interference_size’ [-Winterference-size]`, it is caused by defining alignment that is not portable, you should not worry about it as programs are compiled with `-march-native` flag either way.

## Executables and Usage

The build process produces two executables: mdp_program (Part 1) and qlearning_program (Part 2). Both programs share the same command-line interface format.

### Command-Line Arguments (Flags)
```
./build/<program_name> <filename> [gamma] [epsilon]
```

- `filename` (Required): Path to the world configuration file (e.g., `data/world0.txt`).

- `gamma` (Optional): Discounting factor. Overrides the G parameter in the data file. Defaults to 1.0 if missing entirely.

- `epsilon` (Optional): Exploration parameter for the Q-learning agent. Overrides the E parameter in the data file. Defaults to 0.05 if missing entirely.

_Example execution_:
```
./build/qlearning_program data/world1.txt 0.99 0.25
```

### Environmental Variables (Q-Learning)

The Q-learning simulator (`qlearning_program`) is heavily multi-threaded. You can control the concurrency and training volume using the following environment variables. If omitted, the program dynamically calculates sensible defaults based on your hardware.

- `MDP_NUM_THREADS`

  - Description: Target number of concurrent worker threads.

  - Default: Automatically scales to `std::thread::hardware_concurrency()` (or 4 if undetected), and is clamped dynamically based on `MDP_TOTAL_TRIALS` to ensure each thread has enough work.

- `MDP_EPOCHS`

  - Description: Number of synchronization points (barrier drops) where threads merge their local Q-tables using exact federated averaging.

  - Default: `100`

- `MDP_TOTAL_TRIALS`

  - Description: The absolute total number of training episodes to simulate distributed across all threads.

  - Default: `1000000`

_Example execution with custom environment configuration_:

```
MDP_NUM_THREADS=48 MDP_TOTAL_TRIALS=10000000000 ./build/qlearning_program data/world0.txt
```

## Generating Convergence Graphs

A unified Gnuplot script (`plot_convergence.gp`) is provided to visualize the algorithmic convergence of both programs. Make sure to run the respective program first to generate the necessary data files (convergence.dat or `qlearning_convergence.dat`).

The script takes a command-line argument to determine the plotting mode:

### For Part 1 (Value Iteration MDP):

```
gnuplot -c plot_convergence.gp mdp
```

(This produces `mdp_convergence.png` plotting iterations vs. max utility delta)

### For Part 2 (Q-Learning RL):

```
gnuplot -c plot_convergence.gp ql
```

(This produces `qlearning_convergence.png` mapping total distributed trials vs. delta on a multi-field log-log scale)

## Appendix: Extreme Scale Benchmarks

Note: The test that separates the boys from the men.