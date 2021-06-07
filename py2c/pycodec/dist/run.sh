# !/bin/bash

#export LIBVA_DRIVERS_PATH=/opt/intel/mediasdk/lib:/usr/lib/x86_64-linux-gnu/dri:/usr/local/lib
#export LIBVA_DRIVERS_PATH=/opt/intel/mediasdk/lib:/usr/local/lib/dri/:/usr/local/lib
#export LIBVA_DRIVERS_PATH=/opt/intel/mediasdk/lib:`pwd`:/usr/local/lib
#export LIBVA_DRIVERS_PATH=`pwd`/back:/usr/local/lib/dri/
#export LIBVA_DRIVERS_PATH=`pwd`/back:`pwd`/lib
export LIBVA_DRIVERS_PATH=`pwd`
export LD_LIBRARY_PATH=$LIBVA_DRIVERS_PATH

echo $LIBVA_DRIVERS_PATH
echo $LD_LIBRARY_PATH
./pyui2
