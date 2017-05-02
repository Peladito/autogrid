dir_original=`pwd`
iam=`whoami`

cd ./autogrid_dock_src/autogrid
make 
cd $dir_original
mv ../autogrid_dock_src/autogrid/autogrid4 ./exe/autogrid

cd ./out
../exe/autogrid -p ../input/ori_rec_ali_1gic-A.gpf -w "../input/biasM_M-BSCWS.dat" > out.log
#mv ./rec_ali_1gic-A.OA.map ./modificado-A.OA.map
#grep WS out | wc -l
#diff ./original.OA.map ./modificado-A.OA.map
