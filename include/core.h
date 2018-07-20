/*
  File name: core.h
  written by: Mouhsen Ibrahim <mohsen47@hotmail.co.uk>
  created at: 13:24 2018-01-18 UTC +3
  Description:
    This file defines a systemc model which is used as a high level abstraction
    for a CPU core, it will be used to model a multicore architecture.
*/
#ifndef _CORE_H
#define _CORE_H

#include <systemc.h>
#include <task.h>
#include <types.h>
#include <execute.h>
#include <parameters.h>
#include <utils.h>

SC_MODULE(core) {
  sc_in_clk clk;
  sc_in<task> t_in; // Task input
  sc_in<bool> t_in_v; // Is task input valid?
  sc_out<bool> t_in_f; // Finished reading input task?
  sc_out<task> t_out; // Task output
  sc_out<bool> t_out_v; // Is task output valid?
  sc_in<bool> t_out_f; // Finished reading output task?
  //sc_out<bool> finished;
  sc_out<bool> rdy; // Is core ready to receive new tasks?

  execute* ex;
  sc_signal<bool> rdy_sig;
  sc_signal<task> t_in_sig;
  sc_signal<bool> t_in_v_sig;
  sc_signal<bool> t_in_f_sig;
  sc_signal<task> t_out_sig;
  sc_signal<bool> t_out_v_sig;
  sc_signal<bool> t_out_f_sig;
  //sc_signal<bool> finished_sig;

  //task ts[BUFFER_DEPTH];
  sc_fifo<task> taskFifo;
  int num_tasks;
  task previous_task;
  bool core_rdy;

  void prepare(); // Read a task and add it to the FIFO.
  void send_task(); // Read a task from FIFO, fetch its arguments and send it to execution unit
  void handle_finished(); // Read the finished task from execution unit.

  SC_CTOR(core) {

    rdy.initialize(true);
    t_out_v.initialize(false);
    //finished.initialize(false);
    previous_task.id = 0;

    // initialize the task FIFO
    sc_fifo<task> taskFifo (BUFFER_DEPTH);

    PRINTL("new core %s", name());

    ex = new execute("execute");
    ex->rdy(rdy_sig);
    ex->t_in(t_in_sig);
    ex->t_in_v(t_in_v_sig);
    ex->t_in_f(t_in_f_sig);
    //ex->finished(finished_sig);
    ex->t_out(t_out_sig);
    ex->t_out_v(t_out_v_sig);
    ex->t_out_f(t_out_f_sig);
    ex->clk(clk);

    num_tasks = 0;
    core_rdy = true;
    SC_CTHREAD(prepare, clk.pos());
    SC_CTHREAD(send_task, clk.pos());
    SC_CTHREAD(handle_finished, clk.pos());
  }
};

#endif
