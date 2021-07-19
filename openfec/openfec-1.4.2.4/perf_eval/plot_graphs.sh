#!/bin/bash
nberr=0
err=0 #boolean : error found ? true = 1 ; false = 0 
for dir in results_*
do
	cd $dir
	echo -e "\n\e[1mPlotting curves in" $dir "...\e[0m" 
	gnuplot *.dem || err=1
	cd ..
	if [ $err -eq 1 ]
	then
		nberr=`expr $nberr + 1`
		echo -e "\e[31m\e[1mError(s)\e[0m\e[31m found(s) in folder $dir\e[0m"
		err=0
	fi
done
#set error line color : green if no error found, red else.
if [ $nberr -eq 0 ]
then
	echo -e "\e[32m"
else
	echo -e "\e[31m"
fi

echo -e $nberr "error(s) found(s)\e[0m\n"
