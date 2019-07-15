#ifndef _NEXUS1__H
#define _NEXUS1__H

#include <systemc.h>

#include <table.h>
#include <types.h>

#include <parameters.h>
#include <utils.h>

namespace nexus1 {
  class KickOfList {
    private:
      std::vector<task> tasks;
      int size;
    public:
      KickOfList() {
        tasks = std::vector<task>();
        size = 0;
      }
      bool push(task t) {
        if ( size < NEXUS1_KICK_OFF_LIST_SIZE ) {
          tasks.push_back(t);
          size++;
          return true;
        }
        return false;
      }
      void pop() {
        if (!tasks.empty()) {
          tasks.pop_back();
          size--;
        }
      }

      bool empty() {
        return tasks.empty();
      }

      void print() {
        for(int i = 0; i < tasks.size(); i++) {
          if (i == 0) {
            std::cout << "Kick Of List:" << std::endl;
          }
          std::cout << "i: " << i << " task: " << tasks[i].id << std::endl;
        }
      }
  };

  class ProducersTableEntry {
    private:
      mem_addr addr;
      KickOfList kick_of_list;
    public:
      ProducersTableEntry(mem_addr addr) {
        this->addr = addr;
        kick_of_list = KickOfList();
      }
      ProducersTableEntry(mem_addr addr, task t) {
        this->addr = addr;
        kick_of_list = KickOfList();
        kick_of_list.push(t);
      }

      mem_addr get_addr() {
        return addr;
      }

      void set_addr(mem_addr addr) {
        this->addr = addr;
      }

      bool add_task(task t) {
        return kick_of_list.push(t);
      }

      void del_task() {
        kick_of_list.pop();
      }

      void print() {
        std::cout << "addr: " << addr << std::endl;
        kick_of_list.print();
      }
  };

  class ProducersTable : public Table<ProducersTableEntry> {
    public:
      ProducersTable(int count) : Table<ProducersTableEntry>(count) {}
      ProducersTableEntry *get_entry_for_addr(mem_addr addr) {
        for (int i = 0; i < count; i++) {
          if (entries[i] != nullptr && entries[i]->get_used() && entries[i]->get_data() && entries[i]->get_data()->get_addr() == addr) {
            return entries[i]->get_data();
          }
        }
        return nullptr;
      }
      bool add_task(mem_addr addr, task t) {
        ProducersTableEntry *pte = this->get_entry_for_addr(addr);
        if (pte != nullptr) {
          return pte->add_task(t);
        }
        return false;
      }
  };

  class ConsumersTableEntry {
    private:
      mem_addr addr;
      int deps;
      KickOfList kick_of_list;
    public:
      ConsumersTableEntry(mem_addr addr, task t) {
        this->addr = addr;
        this->deps = 1;
        this->kick_of_list = KickOfList();
        this->kick_of_list.push(t);
      }

      mem_addr get_addr() {
        return addr;
      }

      void set_addr(mem_addr addr) {
        this->addr = addr;
      }

      int get_deps() {
        return deps;
      }

      bool inc_deps(int i = 1) {
        if (kick_of_list.empty()) {
          this->deps += i;
          return true;
        }
        return false;
      }

      bool add_task(task t) {
        return kick_of_list.push(t);
      }

      bool empty() {
        return kick_of_list.empty();
      }

      void del_task() {
        kick_of_list.pop();
      }

      void print() {
        std::cout << "addr: " << addr << " deps: " << deps << std::endl;
        kick_of_list.print();
      }
  };

  class ConsumersTable : public Table<ConsumersTableEntry> {
    public:
      ConsumersTable(int count) : Table<ConsumersTableEntry>(count) {}
      ConsumersTableEntry *get_entry_for_addr(mem_addr addr) {
        for (int i = 0; i < count; i++) {
          if (entries[i] != nullptr && entries[i]->get_used() && entries[i]->get_data() && entries[i]->get_data()->get_addr() == addr) {
            return entries[i]->get_data();
          }
        }
        return nullptr;
      }
      bool add_task(mem_addr addr, task t) {
        ConsumersTableEntry *cte = this->get_entry_for_addr(addr);
        if (cte != nullptr) {
          return cte->add_task(t);
        }
        return false;
      }
      bool is_kick_of_list_empty(mem_addr addr) {
        ConsumersTableEntry *cte = this->get_entry_for_addr(addr);
        if (cte != nullptr) {
          return cte->empty();
        }
        return true;
      }
  };

  enum TaskStatus { NEW, SENT, FINISHED };
  static std::string status_array[] = {"NEW", "SENT", "FINISHED"};
  class TaskTableEntry {
    private:
      task t;
      TaskStatus status;
      int deps;
    public:
      TaskTableEntry(task t) {
        this->t = t;
        status = NEW;
        deps = 0;
      }
      task get_task() {
        return t;
      }
      void set_task(task t) {
        this->t = t;
      }
      TaskStatus get_status() {
        return status;
      }
      void set_status(TaskStatus new_status) {
        this->status = new_status;
      }
      int get_deps() {
        return this->deps;
      }
      void set_deps(int deps) {
        this->deps = deps;
      }
      bool is_ready() {
        return this->deps == 0;
      }
      void print() {
        std::cout << "task id: " << t.id << " status: " << status_array[this->status] << " deps: " << this->deps << std::endl;
      }
  };

  class TaskTable : public Table<TaskTableEntry> {
    public:
      TaskTable(int count) : Table<TaskTableEntry>(count) {}
  };

  class TaskPoolEntry {
    private:
      task t;
    public:
      TaskPoolEntry(task t) {
        this->t = t;
      }
      task get_task() {
        return this->t;
      }
      void set_task(task t) {
        this->t = t;
      }
      void print() {
        std::cout << "task id: " << t.id << " inputs: " << t.input_args << " outputs: " << t.output_args << std::endl;
      }
  };
  SC_MODULE(nexus) {

    sc_in_clk clk;
    sc_in<task> t_in; // Task input
    sc_in<bool> t_in_v; // Is task input valid?
    sc_out<bool> t_in_f; // Finished reading input task?
    sc_out<task> t_out; // Task output
    sc_out<bool> t_ready; // Is there a ready task for execution?
    sc_out<bool> t_out_v; // Is task output valid?
    sc_in<bool> t_out_f; // Finished reading output task?
    sc_in<task> t_f_in; // Finished task
    sc_out<bool> t_f_in_f; // Finished reading finished task?
    sc_in<bool> t_f_in_v; // Finished task input valid?
    sc_out<bool> rdy; // Check if nexus is ready or not to receive tasks?

    #ifdef DEBUG
    sc_in<int> debug;
    void debug_thread();
    void debug_print(int);
    #endif

    sc_fifo<task> in_buffer; // Buffer for received tasks.
    sc_fifo<task> task_queue; // Buffer for tasks ready for execution.

    Table<TaskPoolEntry>* task_pool;
    //Table<TaskTableEntry>* task_table;
    TaskTable* task_table;
    ProducersTable* producers_table;
    ConsumersTable* consumers_table;

    task previous_task;
    task previous_f_task;

    // Nexus1 threads
    void receive(); // Receive a new task and store it in receive buffer
    void load(); // Load a task from receive buffer to Task Pool and other tables
    //void schedule(); // Find the next ready task for execution and send it to ready queue
    void add_to_task_table(task*);
    int calculate_deps(task*);
    void send_task();

    void read_finished(); // Read finished tasks and delete them.
    //void delete_task(task&);

    SC_CTOR(nexus): in_buffer("in_buffer", NEXUS1_IN_BUFFER_DEPTH), task_queue("task_queue", NEXUS1_READY_QUEUE_DEPTH) {
      rdy.initialize(true);
      t_out_v.initialize(false);
      t_ready.initialize(false);
      #ifdef DEBUG
      SC_CTHREAD(debug_thread, clk.pos());
      #endif
      previous_task.id = 0;
      previous_f_task.id = 0;

      task_pool = new Table<TaskPoolEntry>(NEXUS1_TASK_NUM);
      task_table = new TaskTable(NEXUS1_TASK_TABLE_SIZE);
      producers_table = new ProducersTable(NEXUS1_PRODUCERS_TABLE_SIZE);
      consumers_table = new ConsumersTable(NEXUS1_CONSUMERS_TABLE_SIZE);

      PRINTL("new nexus 1 %s", name());
      SC_CTHREAD(receive, clk.pos());
      SC_CTHREAD(load, clk.pos());
      SC_CTHREAD(send_task, clk.pos());
      //SC_CTHREAD(schedule, clk.pos());
      SC_CTHREAD(read_finished, clk.pos());

    }
  };
};
#endif
