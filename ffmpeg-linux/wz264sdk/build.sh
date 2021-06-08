#!/bin/bash
set -e

cur=$(cd $(dirname ${BASH_SOURCE[0]}) && pwd)
ffm=${1:-$cur/ffmpeg-4.3.2}
src=$cur/src
tmp=$cur/tmp
out=$cur/out

# 获取绝对路径
ffm=$(readlink -f ${ffm/%\//})
[ -d "$ffm" ] || { echo "usage:$0 [ffmpeg-4.3.2 dir]"; exit 1; }

rm -rf $tmp $out
mkdir -p $tmp

cd $tmp
cp -r $ffm/* $tmp
cp -r $src/* $tmp

$tmp/configure --prefix=$out      \
  --extra-cflags="-I$cur/include" \
  --extra-ldflags="-L$cur/lib"    \
  --enable-libwz264               \
  --enable-ffmpeg                 \
  --enable-gpl                    \
  --enable-version3               \
  --enable-nonfree                \
  --enable-pthreads               \
  --disable-doc \
  || { tail -100 $tmp/ffbuild/config.log; exit 1; }

make -j 4

type -p tree && tree $out
