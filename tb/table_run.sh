#!/bin/bash

clear
g++ -D DEBUG -I. -I$SYSTEMC_HOME/include -I./include -I../include -L. -L$SYSTEMC_HOME/lib-linux64 -L../lib -lsystemc -lm -lstdc++ table_tb.cpp ../src/task.cpp ../src/utils.cpp -o tb_table && ./tb_table
