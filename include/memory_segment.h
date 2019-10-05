/*
  File name: memory.h
  created by: Mouhsen Ibrahim <mohsen47@hotmail.co.uk>
  created at: 21:25 2018-07-25 UTC +3
  Description:
    This is a systemc model which simulates a memory controller to fetch arguments
    from main memory.
*/

#ifndef _MEMORY_SEGMENT_H
#define _MEMORY_SEGMENT_H

#include <systemc.h>
#include <task.h>
#include <types.h>
#include "parameters.h"
#include <cache.h>

#include "utils.h"

SC_MODULE(memory_segment) {
  sc_in_clk clk;
  // Read memory address
  sc_in<bool> core_addr_v;
  sc_out<bool> core_addr_f;
  sc_in<mem_addr> core_addr;
  // Write/read data
  sc_inout<bool> core_data_v;
  sc_inout<bool> core_data_f;
  sc_inout<sc_int<32> > core_data;
  // operation type
  sc_in<bool> core_rw; // true means read and false means write
  //sc_out<bool> core_memory_request;
  //sc_in<bool> core_memory_accept;

  // Read remote memory address
  sc_inout<bool> remote_addr_v;
  sc_inout<bool> remote_addr_f;
  sc_inout<mem_addr> remote_addr;
  // Write/read data
  sc_inout<bool> remote_data_v;
  sc_inout<bool> remote_data_f;
  sc_inout<sc_int<32> > remote_data;
  // operation type
  sc_inout<bool> remote_rw; // true means read and false means write
  // If data not available in cache read from l2cache
  sc_in<bool> memory_request;
  sc_out<bool> memory_accept;
  sc_out<bool> bus_memory_request;
  sc_in<bool> bus_memory_accept;
  // Read remote memory address
  sc_inout<bool> bus_addr_v;
  sc_inout<bool> bus_addr_f;
  sc_inout<mem_addr> bus_addr;
  // Write/read data
  sc_inout<bool> bus_data_v;
  sc_inout<bool> bus_data_f;
  sc_inout<sc_int<32> > bus_data;
  // operation type
  sc_inout<bool> bus_rw; // true means read and false means write

  mem_addr min_addr;
  mem_addr max_addr;
  int index;

  void core_receive();
  void bus_receive();

  int find_memory_segment_index(mem_addr);

  SC_HAS_PROCESS(memory_segment);

  memory_segment(sc_module_name n, mem_addr mi = (mem_addr)0, mem_addr ma = (mem_addr)MEMORY_SEGMENT_SIZE, int in = 0) : sc_module(n) {
    min_addr = mi;
    max_addr = ma;
    index = in;
    memory_accept.initialize(false);
    bus_memory_request.initialize(false);
    SC_CTHREAD(core_receive, clk.pos());
    SC_CTHREAD(bus_receive, clk.pos());
    PRINTL("new memory segment %s with min address %d, max address %d and index %d", name(), min_addr, max_addr, index);
  }
};

SC_MODULE(memory_bus) {
  sc_in_clk clk;
  // Read address
  sc_in<mem_addr> addr;
  sc_in<bool> addr_v;
  sc_out<bool> addr_f;
  // Read/write data
  sc_inout<sc_int<32> > data;
  sc_inout<bool> data_v;
  sc_inout<bool> data_f;
  sc_in<bool> rw;

  sc_vector<sc_out<bool> > memory_request_s;
  sc_vector<sc_in<bool> > memory_request_r;
  sc_vector<sc_in<bool> > memory_accept_r;
  sc_vector<sc_out<bool> > memory_accept_s;

  // Read address
  sc_out<mem_addr> remote_addr;
  sc_out<bool> remote_addr_v;
  sc_in<bool> remote_addr_f;
  // Read/write data
  sc_inout<sc_int<32> > remote_data;
  sc_inout<bool> remote_data_v;
  sc_inout<bool> remote_data_f;
  sc_in<bool> remote_rw;

  void receive();

  SC_CTOR(memory_bus) {
    addr_f.initialize(false);
    data_v.initialize(false);
    data_f.initialize(false);
    remote_addr_v.initialize(false);
    remote_data_v.initialize(false);
    remote_data_f.initialize(false);

    memory_request_s.init(NUMA_NODES);
    memory_request_r.init(NUMA_NODES);
    memory_accept_s.init(NUMA_NODES);
    memory_accept_r.init(NUMA_NODES);
    for (int i = 0; i < NUMA_NODES; i++) {
      memory_request_s[i].initialize(false);
      memory_accept_s[i].initialize(false);
    }
    SC_CTHREAD(receive, clk.pos());
  }
};

#endif
