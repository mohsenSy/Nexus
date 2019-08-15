#!/bin/bash

clear
g++ -g -D DEBUG -I. -I$SYSTEMC_HOME/include -I./include -I../include -L. -L$SYSTEMC_HOME/lib-linux64 -L../lib -lsystemc -lm -lstdc++ nexus2_tb.cpp ../src/task.cpp ../src/nexus2.cpp ../src/utils.cpp -o tb_nexus2 && ./tb_nexus2
