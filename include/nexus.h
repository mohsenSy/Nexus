/*
  File name: nexus.h
  written by: Mouhsen Ibrahim <mohsen47@hotmail.co.uk>
  created at: 17:21 2018-11-02 UTC +3
  Description:
    This file contains the systemc modules for modelling nexus in all its version
    nexus 1, nexus 2 and nexus 3.
*/
#ifndef __NEXUS__H
#define __NEXUS__H

#include <systemc.h>
#include <task.h>
#include <types.h>
#include <parameters.h>
#include <utils.h>

class TableEntry {
  private:
    bool used; // true when the entry is used
    int id;
    void* data; // Data Type for the entry
  public:
    TableEntry(void* entry, int i) {
      data = entry;
      id = i;
      used = false;
    }

    ~TableEntry() {
      data = NULL;
    }

    bool is_used(); // Returns true when the entry is used and false otherwise
    int get_id();
    void* get_data(); // Returns the entry's data

    void set_used(bool); // Sets the used attribute
    void set_id(int);
    void set_data(void*); // Sets the entry's data
};

class Table {
  private:
    TableEntry** entries;
    int count;
  public:
    Table(int c) {
      entries = new TableEntry*[c];
      count = c;
      std::cout << "Created a new table with count " << count << std::endl;
    }

    ~Table() {
      std::cout << "Deleted entries" << std::endl;
      delete entries;
    }

    bool add_entry(void*, int);

    bool delete_entry(int);

    void* get_entry(int);

    void print_entries();

};

SC_MODULE(nexus1) {

  sc_in_clk clk;
  sc_in<task> t_in; // Task input
  sc_in<bool> t_in_v; // Is task input valid?
  sc_out<bool> t_in_f; // Finished reading input task?
  sc_out<task> t_out; // Task output
  sc_out<bool> t_out_v; // Is task output valid?
  sc_in<bool> t_out_f; // Finished reading output task?
  sc_out<bool> rdy; // Check if nexus is ready or not to receive tasks?

  sc_fifo<task> in_buffer; // Buffer for received tasks.
  sc_fifo<task> ready_queue; // Buffer for tasks ready for execution.

  Table* task_pool;

  task previous_task;

  void receive(); // Receive a new task and store it in receive buffer
  void load(); // Load a task from receive buffer to Task Pool and other tables
  void schedule(); // Find the next ready task for execution and send it to ready queue

  SC_CTOR(nexus1): in_buffer(NEXUS1_IN_BUFFER_DEPTH), ready_queue(NEXUS1_READY_QUEUE_DEPTH) {
    rdy.initialize(true);
    t_out_v.initialize(false);
    previous_task.id = 0;

    task_pool = new Table(NEXUS1_TASK_NUM);

    PRINTL("new nexus 1 %s", name());
    SC_CTHREAD(receive, clk.pos());
    SC_CTHREAD(load, clk.pos());
    SC_CTHREAD(schedule, clk.pos());

  }
};

#endif
