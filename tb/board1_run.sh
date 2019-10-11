#!/bin/bash

clear
echo "" > board1.output
rm -f board1.*
g++ -g -D DEBUG -I. -I$SYSTEMC_HOME/include -I./include -I../include -L. -L$SYSTEMC_HOME/lib-linux64 -L../lib -lsystemc -lm -lstdc++ board1_tb.cpp ../src/board1.cpp ../src/task.cpp ../src/execute.cpp ../src/core1.cpp ../src/utils.cpp ../src/memory.cpp ../src/nexus1.cpp ../src/nexus2.cpp ../src/stats.cpp -o tb_board1 && time ./tb_board1 | tee -a board1.output
