# !/bin/bash


export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:`pwd`/libs/

#echo $
#./simpleClient
#./bizCloud

start=`date +%s` #定义脚本运行的开始时间

# thread number can modify if needed
THREAD=4 #并发最大线程数
THREADNUM=4 #总共启动的线程数
ROOTDIR="$(pwd)"
TMP_FIFO="/tmp/$.fifo"

#cannot del here. this is thread control
mkfifo "${TMP_FIFO}" #创建有名管道
exec 6<>"${TMP_FIFO}"   #创建文件描述符6,以可读（<）可写（>）的方式关联管道文件
rm ${TMP_FIFO} #删除管道文件,留下文件描述符

needToDo(){
#do something her you need mutithread do
#echo "do something her you need mutithread do"
#sleep 1
./simpleClient
}

#cannot del here. this is thread control
for ((i=0;i<${THREAD};i++));
do
    echo >&6
done 

#some control 
for ((i=0;i<${THREADNUM};i++))
do
    read -u6
    {
        needToDo
	echo 'success'$i
        echo >&6
    }&
done 
wait

end=`date +%s`  #定义脚本运行的结束时间
echo "TIME:`expr $end - $start`"

exec 6<&- #关闭文件描述符的读
exec 6>&- #关闭文件描述符的写

#exit 0


