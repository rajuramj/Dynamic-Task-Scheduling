
set terminal postscript eps color enhanced "Times" 26
set grid noxtics ytics
set ylabel "Parallel efficiency" font "Times, 26"
set xlabel "Number of tasks per thread" font "Times, 26"
set yrange [0:]
set output "| epstopdf --filter > ../imgs/0327_pareff_vs_taskperthread.pdf"

# legend
set key width -3.5 samplen 1.8
set key bottom right

# margins
set tmargin .5
set rmargin .8
set lmargin 7.5

plot "../outputs/0327_pareff_vs_taskperthread" using ($1/8):($2/$3/8) with lines lt 3 lw 6.0 title "8 threads"

