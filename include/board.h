#ifndef _BOARD_H
#define _BOARD_H
#include <systemc.h>
#include <parameters.h>

#include <task.h>
#include <core.h>
#include <memory.h>
#include <string>


SC_MODULE(board) {
  sc_in_clk clk;
  sc_in<task> t_in;
  sc_in<bool> t_in_v;
  sc_out<bool> t_in_f;
  sc_out<bool> rdy;

  sc_vector<core> cores;
  sc_vector<sc_signal<task> > t_in_sigs;
  sc_vector<sc_signal<bool> > t_in_v_sigs;
  sc_vector<sc_signal<bool> > t_in_f_sigs;
  sc_vector<sc_signal<task> > t_out_sigs;
  //sc_vector<sc_signal<bool, SC_MANY_WRITERS> > t_out_v_sigs;
  sc_vector<sc_signal<bool> > t_out_v_sigs;
  sc_vector<sc_signal<bool> > t_out_f_sigs;
  //sc_vector<sc_signal<bool, SC_MANY_WRITERS> > rdy_sigs;
  sc_vector<sc_signal<bool> > rdy_sigs;
  //sc_vector<sc_signal<bool, SC_MANY_WRITERS> > finished_sigs;

  memory* mem;
  sc_signal<bool, SC_MANY_WRITERS> memory_rdy_sig;
  sc_signal<mem_addr, SC_MANY_WRITERS> memory_addr_sig;
  sc_signal<bool, SC_MANY_WRITERS> memory_addr_v_sig;
  sc_signal<bool, SC_MANY_WRITERS> memory_addr_f_sig;
  sc_signal<bool, SC_MANY_WRITERS> memory_data_rdy_sig;
  sc_vector<sc_signal<bool> > memory_request_sigs;
  sc_vector<sc_signal<bool> > memory_accept_sigs;

  task previous_task;
  sc_fifo<task> taskFifo;

  void receiveTask();
  void sendTask();

  SC_CTOR(board) {
    SC_CTHREAD(receiveTask, clk.pos());
    //SC_THREAD(receiveTask);
    //sensitive << clk;
    SC_CTHREAD(sendTask, clk.pos());
    //SC_THREAD(sendTask);
    //sensitive << clk;
    previous_task.id = 0;

    mem = new memory("memory_controller");
    mem->clk(clk);
    mem->rdy(memory_rdy_sig);
    mem->addr(memory_addr_sig);
    mem->addr_f(memory_addr_f_sig);
    mem->addr_v(memory_addr_v_sig);
    mem->data_rdy(memory_data_rdy_sig);
    memory_request_sigs.init(CORE_NUM);
    memory_accept_sigs.init(CORE_NUM);
    for(int i = 0; i < CORE_NUM; i++) {
      mem->core_memory_request[i](memory_request_sigs[i]);
      mem->core_memory_accept[i](memory_accept_sigs[i]);
    }

    // initialize the task FIFO
    sc_fifo<task> taskFifo (TASK_NUM);
    cores.init(CORE_NUM);
    t_in_sigs.init(CORE_NUM);
    t_in_v_sigs.init(CORE_NUM);
    t_in_f_sigs.init(CORE_NUM);
    t_out_sigs.init(CORE_NUM);
    t_out_v_sigs.init(CORE_NUM);
    t_out_f_sigs.init(CORE_NUM);
    rdy_sigs.init(CORE_NUM);
    //finished_sigs.init(CORE_NUM);
    // Bind signals for all cores
    for (int i = 0; i < CORE_NUM; i++) {
      cores[i].clk(clk);
      cores[i].t_in(t_in_sigs[i]);
      cores[i].t_in_v(t_in_v_sigs[i]);
      cores[i].t_in_f(t_in_f_sigs[i]);
      cores[i].t_out(t_out_sigs[i]);
      cores[i].t_out_v(t_out_v_sigs[i]);
      cores[i].t_out_f(t_out_f_sigs[i]);
      cores[i].rdy(rdy_sigs[i]);
      cores[i].memory_rdy(memory_rdy_sig);
      cores[i].memory_addr(memory_addr_sig);
      cores[i].memory_addr_v(memory_addr_v_sig);
      cores[i].memory_addr_f(memory_addr_f_sig);
      cores[i].memory_data_rdy(memory_data_rdy_sig);
      cores[i].memory_request(memory_request_sigs[i]);
      cores[i].memory_accept(memory_accept_sigs[i]);
    }
  }
};
#endif
