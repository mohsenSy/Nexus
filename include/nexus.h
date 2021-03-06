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

enum TaskStatus { NEW, SENT, FINISHED };

typedef struct task_table_entry {
  int id;
  task* t;
  TaskStatus status;
  int num_of_deps;

}TaskTableEntry;

typedef struct prod_table_entry {
  mem_addr addr;
  int index;
  task kick_of_list[NEXUS1_KICK_OFF_LIST_SIZE];
}ProdTableEntry;

typedef struct cons_table_entry {
  mem_addr addr;
  int num_of_deps;
  int index;
  task kick_of_list[NEXUS1_KICK_OFF_LIST_SIZE];
}ConsTableEntry;

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
  protected:
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
      delete [] entries;
    }

    bool add_entry(void*, int);

    bool delete_entry(int);

    void* get_entry(int);

    TaskTableEntry* get_ready_task();

    void print_entries();

};

class ProducersTable : Table {
  public:
    ProducersTable(int c): Table(c) {}
    prod_table_entry* get_entry(mem_addr);
    bool add_entry(prod_table_entry*);
    //bool delete_addr(mem_addr);
    void print_entries();
    bool add_to_kick_off_list(mem_addr, task);
};

class ConsumersTable : public Table {
  public:
    ConsumersTable(int c): Table(c) {}
    cons_table_entry* get_entry(mem_addr);
    bool add_entry(cons_table_entry*);
    //bool delete_entry(cons_table_entry*);
    bool delete_addr(mem_addr);
    void print_entries();
    void increment_deps(mem_addr);
    bool add_to_kick_off_list(mem_addr, task);
};


SC_MODULE(nexus1) {

  sc_in_clk clk;
  sc_in<task> t_in; // Task input
  sc_in<bool> t_in_v; // Is task input valid?
  sc_out<bool> t_in_f; // Finished reading input task?
  sc_out<task> t_out; // Task output
  sc_out<bool> t_out_v; // Is task output valid?
  sc_in<bool> t_out_f; // Finished reading output task?
  sc_in<task> t_f_in; // Finished task
  sc_out<bool> t_f_in_f; // Finished reading finished task?
  sc_in<bool> t_f_in_v; // Finished task input valid?
  sc_out<bool> rdy; // Check if nexus is ready or not to receive tasks?

  sc_fifo<task> in_buffer; // Buffer for received tasks.
  sc_fifo<task> ready_queue; // Buffer for tasks ready for execution.

  Table* task_pool;
  Table* task_table;
  ProducersTable* producers_table;
  ConsumersTable* consumers_table;

  task previous_task;
  task previous_f_task;

  void receive(); // Receive a new task and store it in receive buffer
  void load(); // Load a task from receive buffer to Task Pool and other tables
  void schedule(); // Find the next ready task for execution and send it to ready queue
  void add_to_task_table(task*);
  int calculate_deps(task*);
  void send_task(TaskTableEntry *t);

  void read_finished(); // Read finished tasks and delete them.
  void delete_task(task&);

  SC_CTOR(nexus1): in_buffer(NEXUS1_IN_BUFFER_DEPTH), ready_queue(NEXUS1_READY_QUEUE_DEPTH) {
    rdy.initialize(true);
    t_out_v.initialize(false);
    previous_task.id = 0;
    previous_f_task.id = 0;

    task_pool = new Table(NEXUS1_TASK_NUM);
    task_table = new Table(NEXUS1_TASK_TABLE_SIZE);
    producers_table = new ProducersTable(NEXUS1_PRODUCERS_TABLE_SIZE);
    consumers_table = new ConsumersTable(NEXUS1_CONSUMERS_TABLE_SIZE);

    PRINTL("new nexus 1 %s", name());
    SC_CTHREAD(receive, clk.pos());
    SC_CTHREAD(load, clk.pos());
    SC_CTHREAD(schedule, clk.pos());
    SC_CTHREAD(read_finished, clk.pos());

  }
};

#endif
