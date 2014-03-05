

set style line 80 lt rgb "#000000"

# Line style for grid
#set style line 81 lt 0  # dashed
#set style line 81 lt rgb "#808080"  # grey

#set grid back linestyle 81
set border 3 back linestyle 80 # Remove border on top and right.  These
             # borders are useless and make it harder
             # to see plotted lines near the border.
    # Also, put it in grey; no need for so much emphasis on a border.
set xtics nomirror
set ytics nomirror



set style line 1 lt rgb "#A00000" lw 4 pt 1 ps 0.5
set style line 2 lt rgb "#00A000" lw 4 pt 5 ps 0.5
set style line 3 lt rgb "#5060D0" lw 4 pt 7 ps 0.5
set style line 4 lt rgb "#FF1493" lw 4 pt 9 ps 0.5
set style line 5 lt rgb "red" lw 4 pt 11 ps 0.5
set style line 6 lt rgb "#808000" lw 4 pt 13 ps 0.5
set style line 7 lt rgb "#00008B" lw 4 pt 15 ps 0.5
set style line 8 lt rgb "#800080" lw 4 pt 21 ps 0.5
set style line 9 lt rgb "black" lw 4 pt 63 ps 0.5
set style line 10 lt rgb "blue" lw 4 pt 28 ps 0.5
set style line 11 lt rgb "violet" lw 4 pt 44 ps 0.5
set style line 81 lt 0  # dashed
set style line 81 lt rgb "#808080"  # grey

set grid back linestyle 81
#set xtics 2
#set ytics 1
set term pdfcairo 
#fontscale 0.8


set out "ratiobitpacking.pdf"

set xlabel "Ratio length (large list) / length(small list)"
set ylabel "relative speed (scalar = 1)"
#set ylabel "relative speed (galloping = 1)"

set key bmargin
set key samplen 2 spacing .5 font ",8" maxrows 4
#set xrange [1:8192]
set xrange [1:10000]
set logscale x 2

set logscale y 2


set out "benchintersection5march2014_gallop.pdf"
#set logscale x 2
#set logscale y

plot "benchintersection5march2014.txt" using 1:($13/$3) ti "gallop" with linespoints lw 2 ps 0.5,\
"" using 1:($21/$3) ti "SIMD gallop" with linespoints lw 2 ps 0.5,\
"" using 1:($22/$3) ti "SIMD gallop2" with linespoints lw 2 ps 0.5,\
"" using 1:($25/$3) ti "Wu SIMD gallop v0" with linespoints lw 2 ps 0.5,\
"" using 1:($26/$3) ti "Wu SIMD gallop v1" with linespoints lw 2 ps 0.5,\
"" using 1:($27/$3) ti "Wu SIMD gallop v2" with linespoints lw 2 ps 0.5,\
"" using 1:($28/$3) ti "Wu SIMD gallop v3" with linespoints lw 2 ps 0.5


set out "benchintersection5march2014_v1.pdf"

plot "benchintersection5march2014.txt" using 1:($34/$3) ti "SIMD v1" with linespoints lw 2 ps 0.5,\
"" using 1:($29/$3) ti "Wu SIMD v1" with linespoints lw 2 ps 0.5,\
"" using 1:($30/$3) ti "Wu SIMD v1 plow" with linespoints lw 2 ps 0.5



set out "benchintersection5march2014_v3.pdf"

plot "benchintersection5march2014.txt" using 1:($35/$3) ti "SIMD v3" with linespoints lw 2 ps 0.5,\
"" using 1:($32/$3) ti "Wu SIMD v3" with linespoints lw 2 ps 0.5,\
"" using 1:($33/$3) ti "Wu SIMD v3 aligned" with linespoints lw 2 ps 0.5
