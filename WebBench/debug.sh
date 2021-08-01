#! /bin/bash

cd /mnt/hgfs/test/build/Debug/WebServer
./WebServer&
cd /mnt/hgfs/test/WebServer-master
cd WebBench
./webbench -f -c 2 -t 10 http://192.168.43.39/index.html
