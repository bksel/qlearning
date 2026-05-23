# AIML Project 4 -- Markov's Decision problem


Test, który odróżnia dzieci od mężczyzn:
```
MDP_TOTAL_TRIALS=10000000000 time build/qlearning_program data/world0.txt
```


Biggest run on panamint:
```
bksel@panamint:~/stopien2/aiml/project4
$ time MDP_TOTAL_TRIALS=100000000000 ./build/qlearning_program data/world0.txt 
Author: Bartosz Ksel (275472)
Starting Q-Learning Experiment:
  Threads: 64
  Epochs: 100
  Total Trials: 100000000000

Running epoch (100/100)...
High-Speed Q-Learning completed! Data written to qlearning_convergence.dat

Utilities:
+----------+----------+----------+----------+
|   0.8508 |   0.9077 |   0.9578 |   1.0000 |
+----------+----------+----------+----------+
|   0.7997 |    F     |   0.7001 |  -1.0000 |
+----------+----------+----------+----------+
|   0.7393 |   0.6834 |   0.6314 |   0.3892 |
+----------+----------+----------+----------+

Policy:
+---+---+---+---+
| > | > | > | T |
+---+---+---+---+
| ^ | F | ^ | T |
+---+---+---+---+
| ^ | < | < | < |
+---+---+---+---+

real	10m28.603s
user	583m22.071s
sys	0m4.646s

```


bksel@panamint:~/stopien2/aiml/project4
$ time MDP_NUM_THREADS=48 MDP_TOTAL_TRIALS=10000000000 ./build/qlearning_program data/world0.txt 
Author: Bartosz Ksel (275472)
Starting Q-Learning Experiment:
  Threads: 48
  Epochs: 100
  Total Trials: 10000000000

Running epoch (100/100)...
High-Speed Q-Learning completed! Data written to qlearning_convergence.dat

Utilities:
+----------+----------+----------+----------+
|   0.8113 |   0.8678 |   0.9178 |   1.0000 |
+----------+----------+----------+----------+
|   0.7609 |    F     |   0.6602 |  -1.0000 |
+----------+----------+----------+----------+
|   0.7030 |   0.6504 |   0.6059 |   0.3601 |
+----------+----------+----------+----------+

Policy:
+---+---+---+---+
| > | > | > | T |
+---+---+---+---+
| ^ | F | ^ | T |
+---+---+---+---+
| ^ | < | < | < |
+---+---+---+---+

real	1m25.764s
user	59m4.985s
sys	0m1.697s
