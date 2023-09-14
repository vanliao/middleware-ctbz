#!/bin/bash

platform=pc

if [ $# -eq 1 ];then
    platform=arm
fi

export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/opt/buildroot-gcc463/usr/lib32:/opt/buildroot-gcc463/usr/mipsel-buildroot-linux-uclibc/sysroot/lib/

make clean && make platform=${platform}
#make platform=${platform}

#valgrind --track-fds=yes  --leak-check=full ./electricfence
#nginx
#spawn-fcgi -p 8088 -f ./oamCGI
