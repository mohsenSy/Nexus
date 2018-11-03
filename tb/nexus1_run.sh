#!/bin/bash

clear
g++ -D DEBUG -I. -I$SYSTEMC_HOME/include -I./include -I../include -L. -L$SYSTEMC_HOME/lib-linux64 -L../lib -lsystemc -lm -lstdc++ nexus1_tb.cpp ../src/task.cpp ../src/nexus.cpp ../src/utils.cpp -o tb_nexus1 && ./tb_nexus1
