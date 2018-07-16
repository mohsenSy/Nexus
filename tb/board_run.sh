#!/bin/bash

clear
g++ -g -D DEBUG -I. -I$SYSTEMC_HOME/include -I./include -I../include -L. -L$SYSTEMC_HOME/lib-linux64 -L../lib -lsystemc -lm -lstdc++ board_tb.cpp ../src/board.cpp ../src/task.cpp ../src/execute.cpp ../src/core.cpp ../src/utils.cpp -o tb_board && ./tb_board
