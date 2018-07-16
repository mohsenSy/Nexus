/*
  File name: execute.h
  created by: Mouhsen Ibrahim <mohsen47@hotmail.co.uk>
  created at: 13:44 2018-01-18 UTC +3
  Description:
    This is a systemc model which simulates an execution unit inside the core model
    it simply waits a number of time units to simulate execution
*/

#ifndef _EXECUTE_H
#define _EXECUTE_H

#include <systemc.h>
#include <task.h>

#include "utils.h"

SC_MODULE(execute) {
  sc_in<bool> clk;
  sc_in<task> t_in; // Task input
  sc_in<bool> t_in_v; // Is task input valid to be read?
  sc_out<bool> t_in_f; // Finished reading task input?
  sc_out<bool> rdy; // Ready to receive a new task?
  sc_out<task> t_out; // Task output
  sc_out<bool> t_out_v; // Is task output valid?
  sc_in<bool> t_out_f; // Finished reading task output?

  task previous_task;

  void do_execute();

  SC_CTOR(execute) {
    SC_CTHREAD(do_execute, clk.pos());
    //SC_THREAD(do_execute);
    //sensitive << clk;
    PRINTL("new execute unit %s", name());
    previous_task.id = 0;
    rdy.initialize(true);
  }
};

#endif
