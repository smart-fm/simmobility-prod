set terminal pngcairo size 1400,1048 enhanced font 'Verdana,30'
set output 'seg_density.png'

set yrange [0.5:1]
set ylabel "Frequency"
set xlabel "Segment Density (Vehicles per Lane per Km)"

set key bottom



plot\
'basecase-taxi1000.out.txt.csv.7-24.seg_density.cdf.dat'\
	using 1:2 with lines title "Base Case" linecolor rgb"#1b9e77" linewidth 4,\
'DATA/no_PT-Greedy-demand100-taxi1500_SMS1900.out.txt.csv.7-24.seg_density.cdf.dat'\
	using 1:2 with lines title "AMOD introduced, mass transit excluded" linecolor rgb"#d95f02" linewidth 4,\
'DATA/All_Modes-Greedy-demand100-taxi400_SMS800.out.txt.csv.7-24.seg_density.cdf.dat'\
	using 1:2 with lines title "All Modes" linecolor rgb"#7570b3" linewidth 4,\

