#!/bin/bash

clear
g++ -D DEBUG -I. -I$SYSTEMC_HOME/include -I./include -I../include -L. -L$SYSTEMC_HOME/lib-linux64 -L../lib -lsystemc -lm -lstdc++ core_tb.cpp ../src/task.cpp ../src/execute.cpp ../src/core.cpp ../src/utils.cpp ../src/memory.cpp ../src/stats.cpp -o tb_core && ./tb_core
