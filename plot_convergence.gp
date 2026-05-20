# Clear any previous settings
reset

# CRITICAL FIX: Tell Gnuplot NOT to treat '#' as a comment, 
# so it actually reads your header row!
set datafile commentschars ""

# Configure the terminal output
set terminal pngcairo size 1024,768 font "Arial,12"
set output "qlearning_convergence.png"

# Grid configuration for high-fidelity reading across logarithmic orders of magnitude
set grid xtics ytics mxtics mytics ls 12 lc rgb '#dddddd' lt 1 lw 1
set grid back

# Titles and Axis Labels
set title "Q-Learning Grid World Convergence Profile\n(Log-Log Multi-Field Tracking Under α = 1/N)" font "Arial-Bold,14"
set xlabel "Total Global Trials Distributed (Log Scale)" font "Arial-Bold,12"
set ylabel "Absolute Value Iteration Delta (Δ Utility, Log Scale)" font "Arial-Bold,12"

# --- LOG-LOG AXES CONFIGURATION ---
set logscale x 10
set logscale y 10

# Formatting axis numbers nicely for logarithmic scales
set format x "10^{%L}"
set format y "10^{%L}"

# Border configurations (Clean plotting frame)
set border 3 back lt 1 lc rgb '#444444' lw 1.5
set tics nomirror

# Legend (Key) styling - neatly boxed on the bottom-left to not block the downwards curves
set key bottom left box lt 1 lc rgb '#bbbbbb' spacing 1.3 opaque

# --- AUTOMATIC MASS PLOTTING LOOP WITH CLEAN LABELS ---
# Read column count dynamically
stats "qlearning_convergence.dat" nooutput
total_columns = STATS_columns

# Plot loop: slices strings like "D(1,1)" into pure coordinate layout "(1,1)"
plot for [col=2:total_columns] "qlearning_convergence.dat" using 1:col \
     with lines linewidth 2 \
     title sprintf("(%s", columnheader(col)[3:strlen(columnheader(col))])