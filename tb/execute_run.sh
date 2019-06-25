#!/bin/bash

clear
g++ -D DEBUG -I. -I$SYSTEMC_HOME/include -I./include -I../include -L. -L$SYSTEMC_HOME/lib-linux64 -lsystemc -lm -lstdc++ execute_tb.cpp ../src/task.cpp ../src/execute.cpp ../src/utils.cpp ../src/stats.cpp -o tb_execute && ./tb_execute
