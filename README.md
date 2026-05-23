# AIML Project 4 -- Markov's Decision Problem

This project implements solvers for discrete Markov Decision Problems (MDP) using direct methods (Value Iteration) and Reinforcement Learning (Q-Learning).

## Building the Project

Ninja is used by default as the build generator, but you can change it to Make by specifying `GENERATOR=make`, (you should not change it unless you do not have Ninja installed).

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

_Not really necessary in order to run programs, just interesting observations._

Known-model approach for reference:
```
~/P/s/a/project4 (main|+1) $ build/mdp_program data/world0.txt
Author: Bartosz Ksel (275472)
Value Iteration converged in 22 iterations.

Policy:
+---+---+---+---+
| > | > | > | T |
+---+---+---+---+
| ^ | F | ^ | T |
+---+---+---+---+
| ^ | < | < | < |
+---+---+---+---+

Utilities:
+----------+----------+----------+----------+
|   0.8116 |   0.8678 |   0.9178 |   1.0000 |
+----------+----------+----------+----------+
|   0.7616 |    F     |   0.6603 |  -1.0000 |
+----------+----------+----------+----------+
|   0.7053 |   0.6553 |   0.6114 |   0.3878 |
+----------+----------+----------+----------+
```

```
bksel@panamint:~/stopien2/aiml/aiml-project4-markov-decision-problem
$ time MDP_TOTAL_TRIALS=10000000000 ./build/qlearning_program data/world0.txt 
Author: Bartosz Ksel (275472)
Starting Q-Learning Experiment:
  Threads: 64
  Epochs: 100
  Total Trials: 10000000000

Running epoch (100/100)...
High-Speed Q-Learning completed! Data written to qlearning_convergence.dat

Utilities:
+----------+----------+----------+----------+
|   0.8113 |   0.8678 |   0.9178 |   1.0000 |
+----------+----------+----------+----------+
|   0.7608 |   0.0000 |   0.6603 |  -1.0000 |
+----------+----------+----------+----------+
|   0.7025 |   0.6492 |   0.6040 |   0.3620 |
+----------+----------+----------+----------+

Policy:
+---+---+---+---+
| > | > | > | T |
+---+---+---+---+
| ^ | * | ^ | T |
+---+---+---+---+
| ^ | < | < | < |
+---+---+---+---+

Q-Values (1 decimal place):
+---------------+---------------+---------------+---------------+
|    ^  0.8     |    ^  0.9     |    ^  0.9     |               |
| < 0.8   > 0.9 | < 0.8   > 0.9 | < 0.9   > 1.0 |     T  1.0    |
|    v  0.8     |    v  0.9     |    v  0.7     |               |
+---------------+---------------+---------------+---------------+
|    ^  0.8     |               |    ^  0.7     |               |
| < 0.8   > 0.8 |       F       | < 0.7   >-0.6 |     T -1.0    |
|    v  0.7     |               |    v  0.4     |               |
+---------------+---------------+---------------+---------------+
|    ^  0.7     |    ^  0.6     |    ^  0.6     |    ^ -0.7     |
| < 0.7   > 0.7 | < 0.7   > 0.6 | < 0.6   > 0.4 | < 0.4   > 0.2 |
|    v  0.7     |    v  0.6     |    v  0.6     |    v  0.3     |
+---------------+---------------+---------------+---------------+

real	1m17.106s
user	67m45.242s
sys	0m0.672s
```
Note, practical gain: $\frac{(67×60)+45}{60+17} \approx 52.79 $ 

Theoretical gain: $64$ 

CPU utilization: $52.79 / 64 = 82.49% $

### Raw Throughput:

- Wall-clock Processing Speed: $\frac{10,000,000,000}{77.106} \approx \textbf{129.69 Million trials/sec}$

- Per-Core Processing Speed: $\frac{10,000,000,000}{4065.242} \approx \textbf{2.46 Million trials/sec}$

### Overcoming the Value Trap

An interesting behavioral observation derived from these extreme-scale benchmarks is the agent's ability to eventually overcome local optima (or "value traps") at specific fields. With smaller trial counts (e.g., millions), the $\epsilon$-greedy exploration strategy often fails to adequately explore deep, low-probability trajectories, causing the agent to settle for a suboptimal policy in hard-to-reach states. However, when scaling $N$ beyond $10,000,000,000$, the law of large numbers takes over. The sheer brute-force volume of random exploration guarantees that even the most statistically improbable state-action sequences are traversed enough times. This allows the $1/N$ learning rate to definitively pull the Q-values out of local minima, forcing convergence toward the true globally optimal paths.

