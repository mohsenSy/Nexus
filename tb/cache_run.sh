#!/bin/bash

clear
echo "" > cache.output
rm cache1.*
g++ -g -D DEBUG -I. -I$SYSTEMC_HOME/include -I./include -I../include -L. -L$SYSTEMC_HOME/lib-linux64 -L../lib -lsystemc -lm -lstdc++ cache_tb.cpp ../src/board.cpp ../src/task.cpp ../src/execute.cpp ../src/core.cpp ../src/utils.cpp ../src/memory.cpp ../src/nexus1.cpp ../src/nexus2.cpp ../src/stats.cpp ../src/cache.cpp -o tb_cache && time ./tb_cache | tee -a cache.output
