#!/bin/bash

# https://yutarine.blogspot.com/2018/02/shell-command-bc02.html
# [コマンド] bc(数値計算ソフト)の結果出力で、小数点前の0(ゼロ)を省略させないようにする方法

#./wls vis.mac name
mv ../output ../output_`date +%Y%m%d_%H%M%S`
mkdir ../output
PWD=`pwd`

NEvents=1

# in cm
CenterX=0;
CenterY=0;
HalfX=0.5;
HalfY=0.5;


MACRO="$PWD/../output/g4surface.mac"

cp ./runsurface.mac $MACRO
sed -i -e "s;###HALFX###;${HalfX};g" $MACRO
sed -i -e "s;###HALFY###;${HalfY};g" $MACRO
sed -i -e "s;###NEVENTS###;${NEvents};g" $MACRO
sed -i -e "s;###CENTERX###;${CenterX};g" $MACRO
sed -i -e "s;###CENTERY###;${CenterY};g" $MACRO

ROOTNAME="$PWD/../output/rootsurface.root"
SCR="../output/g4surface.sh"
echo '#!/bin/bash' > $SCR
echo "#$ -o $PWD/../output/log_X${itX}_Y${itY}.log" >> $SCR
echo "#$ -e $PWD/../output/log_X${itX}_Y${itY}.err" >> $SCR
echo "$PWD/wls $MACRO $ROOTNAME" >> $SCR
echo "$MACRO $ROOTNAME"
#echo qsub $SCR
# bsub -q h -J X${itX}_Y${itY} "./wls $MACRO $ROOTNAME > ../output/log_X${itX}_Y${itY}.log 2>&1"
