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
#include <mutex>

// A mutex to lock and unlock access to memory
extern sc_mutex memory_mutex;

SC_MODULE(core) {
  sc_in_clk clk;
  sc_in<task> t_in; // Task input
  sc_in<bool> t_in_v; // Is task input valid?
  sc_out<bool> t_in_f; // Finished reading input task?
  sc_out<task> t_out; // Task output
  sc_out<bool> t_out_v; // Is task output valid?
  sc_in<bool> t_out_f; // Finished reading output task?
  sc_out<bool> rdy; // Is core ready to receive new tasks?

  sc_out<mem_addr> memory_addr;
  sc_out<bool> memory_addr_v;
  sc_in<bool> memory_addr_f;
  sc_in<bool> memory_rdy;
  sc_in<bool> memory_data_rdy;
  sc_out<bool> memory_request;
  sc_in<bool> memory_accept;

  execute* ex;
  sc_signal<bool> rdy_sig;
  sc_signal<task> t_in_sig;
  sc_signal<bool> t_in_v_sig;
  sc_signal<bool> t_in_f_sig;
  sc_signal<task> t_out_sig;
  sc_signal<bool> t_out_v_sig;
  sc_signal<bool> t_out_f_sig;

  //std::mutex m;
  sc_fifo<task> taskFifo;
  int num_tasks;
  task previous_task;

  int mem_cycles;

  void prepare(); // Read a task and add it to the FIFO.
  void send_task(); // Read a task from FIFO, fetch its arguments and send it to execution unit
  void handle_finished(); // Read the finished task from execution unit.
  void fetch_input(mem_addr);

  bool addTask(task &t);
  bool readTask(task &t);
  int num_free();
  sc_mutex task_fifo_mutex;

  void inc_tasks_num();
  void dec_tasks_num();
  int get_tasks_num();
  sc_mutex tasks_num_mutex;

  SC_CTOR(core): taskFifo(BUFFER_DEPTH)   {
    rdy.initialize(true);
    t_out_v.initialize(false);
    memory_request.initialize(false);
    previous_task.id = 0;
    mem_cycles = 0;
    num_tasks = 0;

    PRINTL("new core %s", name());

    ex = new execute("execute");
    ex->rdy(rdy_sig);
    ex->t_in(t_in_sig);
    ex->t_in_v(t_in_v_sig);
    ex->t_in_f(t_in_f_sig);
    ex->t_out(t_out_sig);
    ex->t_out_v(t_out_v_sig);
    ex->t_out_f(t_out_f_sig);
    ex->clk(clk);

    num_tasks = 0;
    SC_CTHREAD(prepare, clk.pos());
    SC_CTHREAD(send_task, clk.pos());
    SC_CTHREAD(handle_finished, clk.pos());
  }
};

#endif
