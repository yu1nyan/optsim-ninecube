nowdir=`echo ${PWD} | awk -F "/" '{print $NF}'`
echo $nowdir
for((i=0;i<100;i++))
do
 #echo $i
 bsub -q s -J "$nowdir-$i" "../build/wls ./100evt.mac ./$nowdir-$i.root $i"
done
