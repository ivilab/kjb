unset surface
set table '/tmp/contour1.dat'
set contour base
set cntrparam levels incremental -150,10,35
splot 'muller-V' 
unset table
!awk '{if (NF<2){printf "\n"}{print}}' < /tmp/contour1.dat > /tmp/contour2.dat
reset


set term X11
#set term postscript eps enhanced color solid lw 1 "Helvetica" 16 size 6,4
#set output "muller-V.eps"
set title "Muller's Potential"
set key top right
#set xrange [-3:1.5]
set xrange [-2.75:1.75]
set xlabel "x"
set yrange [-1:3]
set ylabel "y"
set zrange [-150:40]
set zlabel "V(x,y)"
set surface
set view 0,0,2.3
#splot '/tmp/muller.out' title "New muller output" with lines lw 1, '/tmp/contour2.dat' notitle with lines lw 1 lc 1
