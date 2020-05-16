set title "xstring"
set xlabel "size level (2^level)"
set ylabel "time (ns)"
set terminal png font " Times_New_Roman,12 "
set terminal png size 1024,768
set output "result.png"
set xtics 0 , 1, 10
set key left

plot \
"result.dat" using 0:2 with linespoints linewidth 2 title "xstring"
