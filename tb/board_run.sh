#!/bin/bash

clear
echo "" > board.output
rm board1.*
g++ -g -D DEBUG -I. -I$SYSTEMC_HOME/include -I./include -I../include -L. -L$SYSTEMC_HOME/lib-linux64 -L../lib -lsystemc -lm -lstdc++ board_tb.cpp ../src/board.cpp ../src/task.cpp ../src/execute.cpp ../src/core.cpp ../src/utils.cpp ../src/cache.cpp ../src/nexus1.cpp ../src/nexus2.cpp ../src/stats.cpp -o tb_board && time ./tb_board | tee -a board.output
