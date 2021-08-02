#! /bin/bash

cd /mnt/hgfs/test/build/Debug/WebServer
./WebServer&
cd /mnt/hgfs/test/WebServer-master/WebBench
./webbench -c 5 -t 5 http://192.168.43.39/index.html
