set title "xstring"
set xlabel "size"
set ylabel "time (ns)"
set terminal png font " Times_New_Roman,12 "
set terminal png size 1024,768
set output "result.png"
set xtics 1024, 8192, 66528
set key left

plot \
"result.dat" using 1:2 with linespoints linewidth 2 title "result"
