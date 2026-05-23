# Clear any previous settings
reset

# CRITICAL FIX: Tell Gnuplot NOT to treat '#' as a comment, 
# so it actually reads your header row!
set datafile commentschars ""

# Configure the terminal output
set terminal pngcairo size 1024,768 font "Arial,12"

# Determine plotting mode from command line argument 'ARG1'. Defaults to 'ql'
mode = exists("ARG1") ? ARG1 : "ql"

if (mode eq "mdp") {
    # ----------------------------------------------------
    # PART 1: VALUE ITERATION CONVERGENCE (MDP)
    # ----------------------------------------------------
    set output "mdp_convergence.png"
    
    set title "Value Iteration Convergence Profile" font "Arial-Bold,14"
    set xlabel "Iterations" font "Arial-Bold,12"
    set ylabel "Maximum Utility Delta (Log Scale)" font "Arial-Bold,12"
    
    set logscale y 10
    set format y "10^{%L}"
    
    set grid xtics ytics mxtics mytics ls 12 lc rgb '#dddddd' lt 1 lw 1
    set border 3 back lt 1 lc rgb '#444444' lw 1.5
    set tics nomirror
    set key top right box lt 1 lc rgb '#bbbbbb' spacing 1.3 opaque

    # MDP data has no headers, just (Iteration, Delta)
    plot "convergence.dat" using 1:2 with lines linewidth 2 lc rgb "blue" title "Max Δ Utility"

} else {
    # ----------------------------------------------------
    # PART 2: Q-LEARNING CONVERGENCE
    # ----------------------------------------------------
    set output "qlearning_convergence.png"
    
    set title "Q-Learning Grid World Convergence Profile\n(Log-Log Multi-Field Tracking Under α = 1/N)" font "Arial-Bold,14"
    set xlabel "Total Global Trials Distributed (Log Scale)" font "Arial-Bold,12"
    set ylabel "Absolute Value Iteration Delta (Δ Utility, Log Scale)" font "Arial-Bold,12"

    # --- LOG-LOG AXES CONFIGURATION ---
    set logscale x 10
    set logscale y 10

    set format x "10^{%L}"
    set format y "10^{%L}"

    set grid xtics ytics mxtics mytics ls 12 lc rgb '#dddddd' lt 1 lw 1
    set border 3 back lt 1 lc rgb '#444444' lw 1.5
    set tics nomirror
    set key bottom left box lt 1 lc rgb '#bbbbbb' spacing 1.3 opaque

    # Read column count dynamically
    stats "qlearning_convergence.dat" nooutput
    total_columns = STATS_columns

    # Plot loop: slices strings like "D(1,1)" into pure coordinate layout "(1,1)"
    plot for [col=2:total_columns] "qlearning_convergence.dat" using 1:col \
         with lines linewidth 2 \
         title sprintf("(%s", columnheader(col)[3:strlen(columnheader(col))])
}