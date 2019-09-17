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

class AddressTableEntry {
  private:
    mem_addr addr;
    int data;
  public:
    AddressTableEntry(mem_addr a, int d = 0) {
      data = d;
      addr = a;
    }
    AddressTableEntry() : addr(0), data(0) {}
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
    friend ostream& operator <<(ostream& out, const AddressTableEntry& ate) {
      out << "addr: " << ate.addr << " data: " << ate.data << endl;
      return out;
    }
    bool operator ==(const AddressTableEntry& ate) {
      return addr == ate.addr;
    }
};

class AddressTable: public Table<AddressTableEntry> {
  public:
    AddressTable(int c) : Table(c) {}
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
  //sc_signal<bool> memory_request;
  //sc_signal<bool> memory_accept;

  int size;
  AddressTable at;

  // Read a memory request from the core
  void receive();

  void read_data();
  void write_data();

  SC_HAS_PROCESS(l1cache);

  l1cache(sc_module_name name, int s = 32) : sc_module(name), at(s) {
    size = s;
    addr_f.initialize(false);
    data_v.initialize(false);
    data_f.initialize(false);
    data.initialize(0);

    SC_CTHREAD(receive, clk.pos());
  }

};

#endif
