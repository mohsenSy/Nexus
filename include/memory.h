/*
  File name: memory.h
  created by: Mouhsen Ibrahim <mohsen47@hotmail.co.uk>
  created at: 21:25 2018-07-25 UTC +3
  Description:
    This is a systemc model which simulates a memory controller to fetch arguments
    from main memory.
*/

#ifndef _MEMORY_H
#define _MEMORY_H

#include <systemc.h>
#include <task.h>
#include <types.h>
#include "parameters.h"

#include "utils.h"

SC_MODULE(memory) {
  sc_in<bool> clk;
  sc_in<mem_addr> addr; // Memory address to fetch
  sc_in<bool> addr_v; // Address is valid?
  sc_out<bool> addr_f; // finished reading memory address
  sc_out<bool> data_rdy; // Data is ready on ouput port (not defined here for simplicity)
  sc_out<bool> rdy; // Ready to receive a new address?
  sc_vector<sc_in<bool> > core_memory_request;
  sc_vector<sc_out<bool> > core_memory_accept;
  sc_mutex mutex;

  void do_fetch();

  SC_CTOR(memory) {
    SC_CTHREAD(do_fetch, clk.pos());
    PRINTL("new memory unit %s", name());
    rdy.initialize(true);
    core_memory_accept.init(CORE_NUM);
    core_memory_request.init(CORE_NUM);
  }
};

#endif
