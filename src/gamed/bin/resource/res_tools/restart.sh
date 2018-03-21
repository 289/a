#!/bin/bash

core_file_name=core.*
tmp_dir=$(date +%Y%m%d_%H:%M:%S)

copy_core_file() {
	for f in $(find . -maxdepth 1 -type f -name "$core_file_name")
	do
		#echo ./cr_bin${f:1}
		mkdir -p ./core_bak/$tmp_dir
		mv $f ./core_bak/$tmp_dir
		cp ./gs ./core_bak/$tmp_dir
	done
}



#echo update svn
cd /home/guest/gameserver/gs/bin/assets
svn up
cd ..
sh gen_version.sh


#echo killall gs and save
cd /home/guest/gameserver/gs/bin
killall -s SIGINT gs
sleep 3



#echo restart bin ......
cp *.log ./log_bak
copy_core_file
./gs gs.conf gmserver.conf gsalias.conf  >1.log 2>&1 &
./gs gs.conf gmserver.conf gsalias2.conf >2.log 2>&1 &
./gs gs.conf gmserver.conf gsalias3.conf >3.log 2>&1 &
./gs gs.conf gmserver.conf gsalias4.conf >4.log 2>&1 &



#echo restart bin2 ......
#cd ../bin2
#cp *.log ./log_bak
#copy_core_file
#./gs gs.conf gmserver.conf gsalias21.conf >1.log 2>&1 &
#./gs gs.conf gmserver.conf gsalias22.conf >2.log 2>&1 &
#./gs gs.conf gmserver.conf gsalias23.conf >3.log 2>&1 &
#./gs gs.conf gmserver.conf gsalias24.conf >4.log 2>&1 &



#echo restart cr_bin ......
cd ../cr_bin
cp *.log ./log_bak
copy_core_file
./gs gs.conf gmserver.conf gsalias_cr.conf 101 >1.log 2>&1 &
./gs gs.conf gmserver.conf gsalias_cr.conf 102 >2.log 2>&1 &
#./gs gs.conf gmserver.conf gsalias_cr.conf 103 >3.log 2>&1 &
#./gs gs.conf gmserver.conf gsalias_cr.conf 104 >4.log 2>&1 &
#./gs gs.conf gmserver.conf gsalias_cr.conf 105 >5.log 2>&1 &


#ps -aux | grep gs | awk '{print $2}' | xargs kill -9
#valgrind --log-file=./valgrind.log --leak-check=full ./gs>1.log 2>&1 &

#killall gs
#./gs >1.log 2>&1 &
#ps -aux | grep gs | awk '{print $2}' | xargs kill -9

#valgrind --log-file=./valgrind.log  --leak-check=full ./gs gs.conf gmserver.conf gsalias.conf  >1.log 2>&1 &
#valgrind --log-file=./valgrind2.log --leak-check=full ./gs gs.conf gmserver.conf gsalias2.conf >2.log 2>&1 &
#valgrind --log-file=./valgrind3.log --leak-check=full ./gs gs.conf gmserver.conf gsalias3.conf >3.log 2>&1 &
#valgrind --log-file=./valgrind4.log --leak-check=full ./gs gs.conf gmserver.conf gsalias4.conf >4.log 2>&1 &


