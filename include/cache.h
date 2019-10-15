/*
  File name: cache.h
  written by: Mouhsen Ibrahim <mohsen47@hotmail.co.uk>
  created at: 22:18 2019-09-17 UTC +3
  Description:
    This file defines two systemc models for level 1 and level 2
    caches, the first level is special for only one core and the
    second one is shared between possibly 8 cores.
*/

#ifndef _CACHE_H
#define _CACHE_H

#include <systemc.h>
#include <iostream>

#include <table.h>
#include <types.h>
#include <parameters.h>

class AddressTableEntry {
  private:
    mem_addr addr;
    int data;
    int time_stamp;
  public:
    AddressTableEntry(mem_addr a, int d = 0) {
      data = d;
      addr = a;
      time_stamp = sc_time_stamp().value();
    }
    AddressTableEntry() : addr(0), data(0), time_stamp(sc_time_stamp().value()) {}
    mem_addr get_addr() {
      return addr;
    }
    void set_addr(mem_addr a) {
      addr = a;
    }

    int get_data() {
      return data;
    }
    void set_data(int d) {
      data = d;
    }
    int get_time_stamp() {
      return time_stamp;
    }
    friend ostream& operator <<(ostream& out, const AddressTableEntry& ate) {
      out << "addr: " << ate.addr << " data: " << ate.data << " time_stamp: " << ate.time_stamp << endl;
      return out;
    }
    bool operator ==(const AddressTableEntry& ate) {
      return addr == ate.addr;
    }
};

class AddressTable: public Table<AddressTableEntry> {
  public:
    AddressTable(int c) : Table(c) {}
    void add_entry(const AddressTableEntry &ate) {
      if (!Table<AddressTableEntry>::add_entry(ate)) {
        int stamp = 0;
        auto entry = begin();
        for (auto it = begin(); it != end(); it++) {
          if (it->get_time_stamp() < stamp) {
            stamp = it->get_time_stamp();
            entry = it;
          }
        }
        delete_entry(*entry);
        Table<AddressTableEntry>::add_entry(ate);
      }
    }
};

SC_MODULE(l1cache) {
  sc_in_clk clk;
  // Read memory address
  sc_in<bool> addr_v;
  sc_out<bool> addr_f;
  sc_in<mem_addr> addr;
  // Write/read data
  sc_inout<bool> data_v;
  sc_inout<bool> data_f;
  sc_inout<sc_int<32> > data;
  // operation type
  sc_in<bool> rw; // true means read and false means write
  // If data not available in cache read from l2cache
  sc_out<bool> memory_request;
  sc_in<bool> memory_accept;

  // l2cache ports
  sc_out<bool> l2_addr_v;
  sc_in<bool> l2_addr_f;
  sc_out<mem_addr> l2_addr;
  // Write/read data
  sc_inout<bool> l2_data_v;
  sc_inout<bool> l2_data_f;
  sc_inout<sc_int<32> > l2_data;
  // operation type
  sc_out<bool> l2_rw; // true means read and false means write

  int size;
  AddressTable at;

  // Read a memory request from the core
  void receive();

  void read_data();
  void write_data();

  int read_data_l2cache(mem_addr);

  SC_HAS_PROCESS(l1cache);

  l1cache(sc_module_name name, int s = 32) : sc_module(name), at(s) {
    size = s;
    addr_f.initialize(false);
    data_v.initialize(false);
    data_f.initialize(false);
    data.initialize(0);
    l2_addr_v.initialize(false);
    l2_data_v.initialize(false);
    l2_data_f.initialize(false);
    l2_data.initialize(0);

    SC_CTHREAD(receive, clk.pos());
  }

  l1cache() = default;

};

SC_MODULE(l2cache) {
  sc_in_clk clk;
  // Read memory address
  sc_in<bool> addr_v;
  sc_out<bool> addr_f;
  sc_in<mem_addr> addr;
  // Write/read data
  sc_inout<bool> data_v;
  sc_inout<bool> data_f;
  sc_inout<sc_int<32> > data;
  // Core memory request lines
  sc_vector<sc_in<bool> > core_memory_request;
  sc_vector<sc_out<bool> > core_memory_accept;
  // operation type
  sc_in<bool> rw; // true means read and false means write
  // If data not available in cache read from the right memory segment
  //sc_signal<bool> memory_request;
  //sc_signal<bool> memory_accept;
  // If data not available in cache read from memory_segment
  // memory_segment ports
  sc_out<bool> memory_segment_addr_v;
  sc_in<bool> memory_segment_addr_f;
  sc_out<mem_addr> memory_segment_addr;
  // Write/read data
  sc_inout<bool> memory_segment_data_v;
  sc_inout<bool> memory_segment_data_f;
  sc_inout<sc_int<32> > memory_segment_data;
  // operation type
  sc_out<bool> memory_segment_rw; // true means read and false means write

  int size;
  AddressTable at;

  // Read a memory request from the core
  void receive();

  void read_data();
  void write_data();

  SC_HAS_PROCESS(l2cache);

  l2cache(sc_module_name name, int s = 320) : sc_module(name), at(s) {
    size = s;
    addr_f.initialize(false);
    data_v.initialize(false);
    data_f.initialize(false);
    memory_segment_addr_v.initialize(false);
    data.initialize(0);
    core_memory_request.init(L2CACHECORENUM);
    core_memory_accept.init(L2CACHECORENUM);

    SC_CTHREAD(receive, clk.pos());
  }
  l2cache() = default;

};

#endif
