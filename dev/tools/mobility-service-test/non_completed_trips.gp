output_file= sprintf("%s.png",data);

set yrange [0:80000]

set terminal pngcairo size 350,262 font 'Verdana,10'
set output output_file

unset key
set xtics rotate by 270
plot data \
using 0:2:xtic(1) with boxes
