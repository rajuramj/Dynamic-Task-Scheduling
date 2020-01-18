
set terminal postscript eps color enhanced "Times" 26
set grid noxtics ytics
set ylabel "Parallel efficiency" font "Times, 26"
set xlabel "Number of threads" font "Times, 26"
set yrange [0:]
set output "| epstopdf --filter > ../imgs/0327_task_granularity_100K.pdf"

# legend
set key width -3.5 samplen 1.8
set key bottom right

# margins
set tmargin .5
set rmargin .8
set lmargin 7.5

plot "../outputs/0327_task_granularity_100K.txt" using 1:2 with linespoints lw 3.0 pt 7 ps 2.0 title "1{/Symbol m}", \
     "" using 1:3 with linespoints lw 3.0 pt 8 ps 2.0 title "3.8{/Symbol m}", \
     "" using 1:4 with linespoints lw 3.0 pt 9 ps 2.0 title "10{/Symbol m}"

