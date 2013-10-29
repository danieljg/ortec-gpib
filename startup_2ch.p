set term wxt size 1280,800 font ",16"
set title 'Two-channel test'
set xlabel 'time [s]'
#set ylabel 'counts'
set xtics font ",18"
set ytics font ",18"
set y2tics font ",18"
set xlabel font ",16"
set key font ",16"
set title font ",16"
set xrange  [0:30]
set yrange  [0:100<*]
set y2range [0:500<*]
set y2tics
#set linetype 1 lc rgb "dark-red" lw 3
#set linetype 2 lc rgb "dark-blue" lw 3
set linetype 1 lc rgb "red" lw 3
set linetype 2 lc rgb "blue" lw 3
set style fill solid .5
set boxwidth 0.43 relative
