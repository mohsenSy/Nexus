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
  sc_vector<sc_inout<bool> > remote_memory_request;
  sc_vector<sc_inout<bool> > remote_memory_accept;

  mem_addr min_addr;
  mem_addr max_addr;
  int index;

  void core_receive();
  void remote_receive();

  int find_memory_segment_index(mem_addr);

  SC_HAS_PROCESS(memory_segment);

  memory_segment(sc_module_name n, mem_addr mi = (mem_addr)0, mem_addr ma = (mem_addr)MEMORY_SEGMENT_SIZE, int in = 0) : sc_module(n) {
    min_addr = mi;
    max_addr = ma;
    index = in;
    remote_memory_accept.init(NUMA_NODES);
    remote_memory_request.init(NUMA_NODES);
    for (int i = 0; i < remote_memory_accept.size(); i++) {
      remote_memory_accept[i].initialize(false);
    }
    SC_CTHREAD(core_receive, clk.pos());
    SC_CTHREAD(remote_receive, clk.pos());
    PRINTL("new memory segment %s with min address %d, max address %d and index %d", name(), min_addr, max_addr, index);
  }
};

#endif
