#!/bin/bash

# run.shのbsubしない版　動作確認用に

# https://yutarine.blogspot.com/2018/02/shell-command-bc02.html
# [コマンド] bc(数値計算ソフト)の結果出力で、小数点前の0(ゼロ)を省略させないようにする方法

#./wls vis.mac name
mv ../output ../output_`date +%Y%m%d_%H%M%S`
mkdir ../output
PWD=`pwd`
# 10 mm cube
NPINTS=6;
OFFSET=0.075 # cm
STEP=`echo "scale=5; (1 - 2*$OFFSET) / ($NPINTS - 1)" | bc`
initX=`echo "scale=5; - 1/2 + $OFFSET" | bc | sed 's/^\./0./'`
initY=`echo "scale=5; - 1/2 + $OFFSET" | bc | sed 's/^\./0./'`

echo "initX=$initX, initY=$initY, STEP=$STEP"

for ((itX=0 ; itX<$NPINTS  ; itX++)); do
   #   if [ $mod -eq 1 ] || [ $mod -eq 2 ] || [ $mod -eq 4 ] || [ $mod -eq 6 ]; then
   #      continue
   #   fi
    for ((itY=0 ; itY<$NPINTS ; itY++)); do
	xposi=`echo "scale=2; $initX + $STEP*$itX" | bc | sed 's/^\./0./'`
	yposi=`echo "scale=2; $initY + $STEP*$itY" | bc | sed 's/^\./0./'`
	echo "x=${xposi}, y=${yposi}"
	MACRO="$PWD/../output/g4mac_X${itX}_Y${itY}.mac"

   	#cp ./run.mac ../output/g4_macro_X${xposi}_Y${yposi}.mac
   	cp ./run.mac $MACRO
   	sed -i -e "s;###XPOSI###;${xposi};g" $MACRO
   	sed -i -e "s;###YPOSI###;${yposi};g" $MACRO

	ROOTNAME="$PWD/../output/root_X${itX}_Y${itY}.root"
	SCR="../output/g4mac_X${itX}_Y${itY}.sh"
	echo '#!/bin/bash' > $SCR
	echo '#$ -S /bin/bash' >> $SCR
	echo '#$ -q tmu' >> $SCR
	echo "#$ -o $PWD/../output/log_X${itX}_Y${itY}.log" >> $SCR
	echo "#$ -e $PWD/../output/log_X${itX}_Y${itY}.err" >> $SCR
	#echo ". /soft/geant4/latest/bin/geant4.sh" >> $SCR
	echo "$PWD/wls $MACRO $ROOTNAME" >> $SCR
	echo "$MACRO $ROOTNAME"
	#echo qsub $SCR
	# bsub -q h -J X${itX}_Y${itY} "./wls $MACRO $ROOTNAME > ../output/log_X${itX}_Y${itY}.log 2>&1"
    done
done
