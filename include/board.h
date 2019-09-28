#ifndef _BOARD_H
#define _BOARD_H
#include <systemc.h>
#include <parameters.h>

#include <task.h>
#include <core.h>
#include <cache.h>
#include <nexus2.h>
#include <string>

using namespace nexus2;

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
  sc_vector<sc_signal<bool> > t_out_v_sigs;
  sc_vector<sc_signal<bool> > t_out_f_sigs;
  sc_vector<sc_signal<bool> > rdy_sigs;

  sc_vector<l1cache> l1s;
  sc_vector<sc_signal<bool, SC_MANY_WRITERS> > memory_addr_v_sigs;
  sc_vector<sc_signal<bool, SC_MANY_WRITERS> > memory_addr_f_sigs;
  sc_vector<sc_signal<mem_addr, SC_MANY_WRITERS> > memory_addr_sigs;
  sc_vector<sc_signal<bool, SC_MANY_WRITERS> > memory_data_v_sigs;
  sc_vector<sc_signal<bool, SC_MANY_WRITERS> > memory_data_f_sigs;
  sc_vector<sc_signal<sc_int<32>, SC_MANY_WRITERS> > memory_data_sigs;
  sc_vector<sc_signal<bool, SC_MANY_WRITERS> > memory_rw_sigs;
  sc_vector<sc_signal<bool, SC_MANY_WRITERS> > l1_memory_request_sigs;
  sc_vector<sc_signal<bool, SC_MANY_WRITERS> > l1_memory_accept_sigs;

  sc_vector<l2cache> l2s;
  sc_vector<sc_signal<bool, SC_MANY_WRITERS> > l2_memory_addr_v_sigs;
  sc_vector<sc_signal<bool, SC_MANY_WRITERS> > l2_memory_addr_f_sigs;
  sc_vector<sc_signal<mem_addr, SC_MANY_WRITERS> > l2_memory_addr_sigs;
  sc_vector<sc_signal<bool, SC_MANY_WRITERS> > l2_memory_data_v_sigs;
  sc_vector<sc_signal<bool, SC_MANY_WRITERS> > l2_memory_data_f_sigs;
  sc_vector<sc_signal<sc_int<32>, SC_MANY_WRITERS> > l2_memory_data_sigs;
  sc_vector<sc_signal<bool, SC_MANY_WRITERS> > l2_memory_rw_sigs;

  // Nexus
  nexus *nex;
  sc_signal<task> t_in_sig;
  sc_signal<bool> t_in_v_sig;
  sc_signal<bool> t_in_f_sig;
  sc_signal<task> t_f_in_sig;
  sc_signal<bool> t_f_in_v_sig;
  sc_signal<bool> t_f_in_f_sig;
  sc_signal<task> t_out_sig;
  sc_signal<bool> t_ready_sig;
  sc_signal<bool> t_out_v_sig;
  sc_signal<bool> t_out_f_sig;
  sc_signal<bool> rdy_sig;
  sc_signal<task> t_ready_out_sig;
  sc_signal<bool> t_ready_out_f_sig;
  sc_signal<bool> t_ready_out_v_sig;
  #ifdef DEBUG
  sc_signal<int> debug_sig;
  #endif

  task previous_task;
  sc_fifo<task> taskFifo;
  sc_fifo<task> ready_queue;

  void receiveTask();
  void sendTask();
  void read_finished();
  void send_ready_tasks();
  void read_ready_tasks();

  void send_task_nexus(task);
  void send_task_core(task);
  void send_finished_nexus(task);

  SC_HAS_PROCESS(board);

  static l1cache* creator_l1cache(const char* name, size_t i) {
    return new l1cache("l1cache" + i);
  }
  static l2cache* creator_l2cache(const char* name, size_t i) {
    return new l2cache("l2cache" + i);
  }

  board(sc_module_name name, bool use_nexus=true) :sc_module(name), l1s("l1cache_vector"), rdy_sigs("rdy_sigs"),
      t_out_f_sigs("t_out_f_sigs"), t_out_v_sigs("t_out_v_sigs"), t_out_sigs("t_out_sigs"), t_in_f_sigs("t_in_f_sigs"),
      t_in_v_sigs("t_in_v_sigs"), t_in_sigs("t_in_sigs"), cores("cores"), l2s("l2cache"){
    SC_CTHREAD(receiveTask, clk.pos());
    SC_CTHREAD(sendTask, clk.pos());
    previous_task.id = 0;

    l1s.init(CORE_NUM, creator_l1cache);
    memory_addr_sigs.init(CORE_NUM);
    memory_addr_v_sigs.init(CORE_NUM);
    memory_addr_f_sigs.init(CORE_NUM);
    memory_data_sigs.init(CORE_NUM);
    memory_data_v_sigs.init(CORE_NUM);
    memory_data_f_sigs.init(CORE_NUM);
    memory_rw_sigs.init(CORE_NUM);
    l1_memory_request_sigs.init(CORE_NUM);
    l1_memory_accept_sigs.init(CORE_NUM);
    l2_memory_addr_sigs.init(CORE_NUM);
    l2_memory_addr_v_sigs.init(CORE_NUM);
    l2_memory_addr_f_sigs.init(CORE_NUM);
    l2_memory_data_sigs.init(CORE_NUM);
    l2_memory_data_v_sigs.init(CORE_NUM);
    l2_memory_data_f_sigs.init(CORE_NUM);
    l2_memory_rw_sigs.init(CORE_NUM);
    for (int i = 0; i < CORE_NUM; i++) {
      l1s[i].clk(clk);
      l1s[i].addr(memory_addr_sigs[i]);
      l1s[i].addr_v(memory_addr_v_sigs[i]);
      l1s[i].addr_f(memory_addr_f_sigs[i]);
      l1s[i].data(memory_data_sigs[i]);
      l1s[i].data_v(memory_data_v_sigs[i]);
      l1s[i].data_f(memory_data_f_sigs[i]);
      l1s[i].rw(memory_rw_sigs[i]);
      l1s[i].memory_request(l1_memory_request_sigs[i]);
      l1s[i].memory_accept(l1_memory_accept_sigs[i]);
      l1s[i].l2_addr(l2_memory_addr_sigs[i]);
      l1s[i].l2_addr_v(l2_memory_addr_v_sigs[i]);
      l1s[i].l2_addr_f(l2_memory_addr_f_sigs[i]);
      l1s[i].l2_data(l2_memory_data_sigs[i]);
      l1s[i].l2_data_v(l2_memory_data_v_sigs[i]);
      l1s[i].l2_data_f(l2_memory_data_f_sigs[i]);
      l1s[i].l2_rw(l2_memory_rw_sigs[i]);
    }

    l2s.init(NUMA_NODES, creator_l2cache);
    int index = 0;
    for (int i = 0; i < NUMA_NODES; i++) {
      //l2s[i] = l2cache("l2cache" + i);
      l2s[i].clk(clk);
      l2s[i].addr(l2_memory_addr_sigs[i]);
      l2s[i].addr_v(l2_memory_addr_v_sigs[i]);
      l2s[i].addr_f(l2_memory_addr_f_sigs[i]);
      l2s[i].data(l2_memory_data_sigs[i]);
      l2s[i].data_v(l2_memory_data_v_sigs[i]);
      l2s[i].data_f(l2_memory_data_f_sigs[i]);
      l2s[i].rw(l2_memory_rw_sigs[i]);
      for (int j = 0; j < L2CACHECORENUM; j++) {
        index = j + i * L2CACHECORENUM;
        std::cout << "index is " << index << std::endl;
        l2s[i].core_memory_accept[j](l1_memory_accept_sigs[i]);
        l2s[i].core_memory_request[j](l1_memory_request_sigs[i]);
      }
      std::cout << "====================" << std::endl;
    }
    if (use_nexus) {
      nex = new nexus("Nexus-1");

      nex->clk(clk);

      nex->t_in(t_in_sig);
      nex->t_in_v(t_in_v_sig);
      nex->t_in_f(t_in_f_sig);

      nex->t_f_in(t_f_in_sig);
      nex->t_f_in_v(t_f_in_v_sig);
      nex->t_f_in_f(t_f_in_f_sig);

      nex->t_out(t_out_sig);
      nex->t_ready(t_ready_sig);
      nex->t_out_v(t_out_v_sig);
      nex->t_out_f(t_out_f_sig);

      nex->rdy(rdy_sig);

      nex->t_ready_out(t_ready_out_sig);
      nex->t_ready_out_f(t_ready_out_f_sig);
      nex->t_ready_out_v(t_ready_out_v_sig);

      #ifdef DEBUG
      nex->debug(debug_sig);
      #endif
    }
    else {
      nex = nullptr;
      SC_CTHREAD(send_ready_tasks, clk.pos());
      SC_CTHREAD(read_ready_tasks, clk.pos());
      SC_CTHREAD(read_finished, clk.pos());
    }

    // initialize the task FIFO
    sc_fifo<task> taskFifo (TASK_NUM);
    sc_fifo<task> ready_queue (READY_QUEUE_NUM);
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
      if (nex) {
        nex->t_ins[i](t_in_sigs[i]);
        nex->t_in_vs[i](t_in_v_sigs[i]);
        nex->t_in_fs[i](t_in_f_sigs[i]);
        nex->t_outs[i](t_out_sigs[i]);
        nex->t_out_vs[i](t_out_v_sigs[i]);
        nex->t_out_fs[i](t_out_f_sigs[i]);
        nex->rdys[i](rdy_sigs[i]);
      }
      cores[i].t_in_v(t_in_v_sigs[i]);
      cores[i].t_in_f(t_in_f_sigs[i]);
      cores[i].t_out(t_out_sigs[i]);
      cores[i].t_out_v(t_out_v_sigs[i]);
      cores[i].t_out_f(t_out_f_sigs[i]);
      cores[i].rdy(rdy_sigs[i]);
      t_out_f_sigs[i].write(false);
      cores[i].memory_addr(memory_addr_sigs[i]);
      cores[i].memory_addr_v(memory_addr_v_sigs[i]);
      cores[i].memory_addr_f(memory_addr_f_sigs[i]);
      cores[i].memory_data(memory_data_sigs[i]);
      cores[i].memory_data_v(memory_data_v_sigs[i]);
      cores[i].memory_data_f(memory_data_f_sigs[i]);
      cores[i].memory_rw(memory_rw_sigs[i]);
    }
  }
};
#endif
