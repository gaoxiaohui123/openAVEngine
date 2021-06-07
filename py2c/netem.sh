#!/bin/sh

#ifconfig
#查看名称可能是eth0或eno1或其他
#tc qdisc show
#删除网卡上的 Netem 配置
#sudo tc qdisc del dev eno1 root
#只能控制发包动作
#add/change/replace/del
#1. 延迟设置
#延迟 300ms ± 100ms 大约30%的包发生延迟
#sudo tc qdisc add dev eth0 root netem delay 300ms 100ms 30%
#sudo tc qdisc del dev eth0 root netem delay 300ms 100ms 30%
 
#2. 丢包
#丢包率 5%
#sudo tc qdisc add dev eno1 root netem loss 5%
#sudo tc qdisc change dev eno1 root netem loss 50%
sudo tc qdisc del dev eno1 root
#
#sudo tc qdisc del dev eno1 root netem loss 15%

#3. 包重复
#sudo tc qdisc add dev eno1 root netem duplicate 1%

#4. 包损坏
#sudo tc qdisc add dev eno1 root netem corrupt 0.2%

#5. 包乱序
#sudo tc qdisc change dev eno1 root netem delay 10ms reorder 25% 50%
#有 25% 的数据包（50%相关）会被立即发送，其他的延迟 10 秒











