#ifndef __NEXUS2_H
#define __NEXUS2_H

#include <table.h>
#include <task.h>
#include <systemc>

namespace nexus2 {
  class TaskPoolEntry {
    private:
      task t;
      bool busy;
      int dc;
      sc_mutex *m;
    public:
      TaskPoolEntry(task &t) {
        this->t = t;
        busy = false;
        dc = 0;
        m = new sc_mutex();
      }
      task getTask() {
        return t;
      }
      bool getBusy() {
        m->lock();
        bool b = busy;
        m->unlock();
        return b;
      }
      void setBusy(bool b) {
        m->lock();
        busy = b;
        m->unlock();
      }
      int getDc() {
        m->lock();
        int d = dc;
        m->unlock();
        return d;
      }
      void setDc(int d) {
        m->lock();
        dc = d;
        m->unlock();
      }
  };

  class TaskPool : public Table<TaskPoolEntry> {
    private:
      int count;
    public:
      TaskPool(int c) : Table<TaskPoolEntry>(c) {}
      bool addTask(task &t) {
        TaskPoolEntry *tpe = new TaskPoolEntry(t);
        return add_entry(tpe, t.id);
      }
  };

  SC_MODULE(nexus) {
    sc_in_clk clk;
    sc_in<task> t_in; // Task input
    sc_in<bool> t_in_v; // Is task input valid?
    sc_out<bool> t_in_f; // Finished reading input task?
    /*sc_out<task> t_out; // Task output
    sc_out<bool> t_ready; // Is there a ready task for execution?
    sc_out<bool> t_out_v; // Is task output valid?
    sc_in<bool> t_out_f; // Finished reading output task?
    sc_in<task> t_f_in; // Finished task
    sc_out<bool> t_f_in_f; // Finished reading finished task?
    sc_in<bool> t_f_in_v; // Finished task input valid?*/
    sc_out<bool> rdy; // Check if nexus is ready or not to receive tasks?
    /*sc_out<task> t_ready_out;
    sc_out<bool> t_ready_out_v;
    sc_in<bool> t_ready_out_f;

    #ifdef DEBUG
    sc_in<int> debug;
    void debug_thread();
    void debug_print(int);
    #endif
    */

    sc_fifo<task> tds_buffer; // Buffer for received tasks.
    sc_fifo<task> new_tasks;
    //sc_fifo<task> task_queue; // Buffer for tasks ready for execution.

    TaskPool* task_pool;
    //TaskTable* task_table;
    //sc_mutex task_table_mutex;
    //ProducersTable* producers_table;
    //ConsumersTable* consumers_table;

    task previous_task;
    //task previous_f_task;

    // Nexus2 threads
    void getTDs(); // Receive a new task and store it in receive buffer
    void writeTP(); // Write tasks to the pool
    /*void add_to_task_table(task*);
    int calculate_deps(task*);
    void send_task();

    int add_input_prod(mem_addr, task*);
    int add_input_cons(mem_addr, task*);
    int add_output_cons(mem_addr, task*);
    int add_output_prod(mem_addr, task*);
    void check_output(mem_addr);
    void check_input(mem_addr);
    bool check_task_input(task, mem_addr);
    bool check_task_output(task, mem_addr);
    void schedule_tasks();
    void send_ready_task();

    void read_finished(); // Read finished tasks and delete them.
    void delete_task(task*);*/
    SC_CTOR(nexus) : tds_buffer("TDs Buffer", NEXUS2_TDS_BUFFER_DEPTH), new_tasks("New Tasks", NEXUS2_NEW_TASKS_NUM){
      t_in_f.initialize(false);
      previous_task.id = 0;
      rdy.initialize(true);
      /*t_out_v.initialize(false);
      t_ready.initialize(false);
      #ifdef DEBUG
      SC_CTHREAD(debug_thread, clk.pos());
      #endif
      previous_f_task.id = 0;
*/
      task_pool = new TaskPool(NEXUS2_TASK_NUM);
      /*task_table = new TaskTable(NEXUS1_TASK_TABLE_SIZE);
      producers_table = new ProducersTable(NEXUS1_PRODUCERS_TABLE_SIZE);
      consumers_table = new ConsumersTable(NEXUS1_CONSUMERS_TABLE_SIZE);

      PRINTL("new nexus 1 %s", name());
      SC_CTHREAD(receive, clk.pos());
      SC_CTHREAD(load, clk.pos());
      //SC_CTHREAD(send_task, clk.pos());
      //SC_CTHREAD(schedule, clk.pos());
      SC_CTHREAD(read_finished, clk.pos());
      SC_CTHREAD(send_ready_task, clk.pos());*/
      SC_CTHREAD(getTDs, clk.pos());
      SC_CTHREAD(writeTP, clk.pos());
    }
  };
}
#endif
