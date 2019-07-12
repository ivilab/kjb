#!/bin/bash -ex
# $Id: spq_histo.bash 20310 2016-02-01 11:32:25Z predoehl $
# This script plots histograms of the demo SPQ paths.
TEMP1=zzz.1.$$
TEMP2=zzz.2.$$
for g in $TEMP1 $TEMP2
do [[ -f $g ]] && echo "temp file $g exists" && exit 1
   touch $g
done
# create list of data files we wish to plot.
# "sink_1" is the orange-colored paths.
for num in -3 -10 -30
do f+=("dist_sto_${num}_sink_1.txt")
done
# create fake "data" for deterministic case by just repeating same deterministic answer over and over
((n=0)) ||true
while read -r
do cat ${f[0]/sto/det}
   ((++n > 150)) && break # stop after 200 for better readbility though less authenticity
done <${f[0]} >$TEMP1
f+=($TEMP1) # add deterministic data file to the list
# process each file
for g in "${f[@]}"
do  printf "# %s\n" "$g"
    awk -v mb=240 -v db=4 -v nb=22 -- 'BEGIN {hb=mb+nb*db; for(i=mb;i<=hb;++i) y[i]=0; } {for(i=mb;i<=hb;++i){if($1<=i){y[i]+=1;break;}} } END {for(i=mb;i<=hb;++i){print i, y[i]*0.001;} }' $g
	echo
	echo
done >$TEMP2
GPE="set style data histogram; "
GPE+="set style histogram; "
GPE+="set term svg; set output 'histo.svg'; "
#GPE+="unset xtics; "
#GPE+="set xtics ; "
#GPE+="set xrange [240:320]; "
#GPE+="unset mxtics; "
GPE+="set ylabel 'Bin relative frequency'; "
GPE+="plot newhistogram 'Path size', "
GPE+=" '$TEMP2' index 0 using 2:xtic(1) title 'beta = 1.5', "
GPE+=" '' index 1 using 2:xtic(1) title 'beta = 5', "
GPE+=" '' index 2 using 2:xtic(1) title 'beta = 15', "
GPE+=" '' index 3 using 2:xtic(1) title 'beta -> inf' ; "
gnuplot -e "$GPE"
rm $TEMP1 $TEMP2

