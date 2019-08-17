#ifndef __NEXUS2_H
#define __NEXUS2_H

#include <table.h>
#include <task.h>
#include <systemc>
#include <utils.h>

namespace nexus2 {
  class KickOfList {
    private:
      std::vector<task> tasks;
      int size;
      sc_mutex* m;
    public:
      KickOfList() {
        tasks = std::vector<task>();
        size = 0;
        m = new sc_mutex();
      }
      bool push(task t) {
        m->lock();
        if ( size < NEXUS1_KICK_OFF_LIST_SIZE ) {
          tasks.push_back(t);
          size++;
          m->unlock();
          return true;
        }
        m->unlock();
        return false;
      }
      void pop() {
        m->lock();
        if (!tasks.empty()) {
          tasks.pop_back();
          size--;
        }
        m->unlock();
      }

      void delete_task(int id) {
        m->lock();
        if (tasks.empty()) {
          m->unlock();
          return;
        }
        if (tasks[0].id == id) {
          tasks.erase(tasks.begin());
          size--;
        }
        m->unlock();
      }

      bool empty() {
        return tasks.empty();
      }

      int get_size() {
        return size;
      }

      task *get_task(int index) {
        m->lock();
        if (index >= 0 && index < size) {
          m->unlock();
          return &tasks[index];
        }
        m->unlock();
        return nullptr;
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

  class DependenceTableEntry {
    private:
      mem_addr addr;
      bool isOut;
      int rdrs;
      bool ww;
      KickOfList list;
      sc_mutex *m;
    public:
      DependenceTableEntry(mem_addr addr, bool isOut = false, int rdrs = 1, bool ww = false) {
        this->addr = addr;
        this->isOut = isOut;
        this->rdrs = rdrs;
        this->ww = ww;
        this->list = KickOfList();
        this->m = new sc_mutex();
      }
      bool getIsOut() {
        m->lock();
        bool b = isOut;
        m->unlock();
        return b;
      }
      bool getWw() {
        m->lock();
        bool b = ww;
        m->unlock();
        return b;
      }
      void incRdrs(int i = 1) {
        m->lock();
        rdrs += i;
        m->unlock();
      }
      void addTask(task &t) {
        list.push(t);
      }
      void setWw(bool b) {
        m->lock();
        ww = b;
        m->unlock();
      }
      void print() {
        cout << "addr " << addr << " mode " << (isOut ? "output" : "input") << " readers " << rdrs << " writer waits " << ww << endl;
      }
  };

  class DependenceTable : public Table<DependenceTableEntry> {
    private:
      int count;
    public:
      DependenceTable(int c) : Table<DependenceTableEntry>(c) {}
      bool addAddr(mem_addr addr, bool isOut = false) {
        DependenceTableEntry *dte = new DependenceTableEntry(addr, isOut);
        return add_entry(dte, *(int *)&addr);
      }
      DependenceTableEntry* getEntry(mem_addr addr) {
        return get_data(*(int *)&addr);
      }
      void dump() {
        print_entries();
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
    sc_out<task> t_ready_out;
    sc_out<bool> t_ready_out_v;
    sc_in<bool> t_ready_out_f;

    #ifdef DEBUG
    sc_in<int> debug;
    void debug_thread();
    void debug_print(int);
    #endif

    sc_fifo<task> tds_buffer; // Buffer for received tasks.
    sc_fifo<task> new_tasks;
    sc_fifo<task> global_ready_tasks;
    sc_vector<sc_fifo<task> > ci_ready_tasks;
    sc_vector<sc_fifo<task> > ci_finished_tasks;
    //sc_fifo<task> task_queue; // Buffer for tasks ready for execution.

    // ports for cores
    sc_vector<sc_out<task> > t_ins;
    sc_vector<sc_out<bool> > t_in_vs;
    sc_vector<sc_in<bool> > t_in_fs;
    sc_vector<sc_in<task> > t_outs;
    sc_vector<sc_in<bool> > t_out_vs;
    sc_vector<sc_out<bool> > t_out_fs;
    sc_vector<sc_in<bool> > rdys;

    TaskPool* task_pool;
    DependenceTable* deps_table;
    //TaskTable* task_table;
    //sc_mutex task_table_mutex;
    //ProducersTable* producers_table;
    //ConsumersTable* consumers_table;

    task previous_task;
    int currentCore;
    //task previous_f_task;

    // Nexus2 threads
    void getTDs(); // Receive a new task and store it in receive buffer
    void writeTP(); // Write tasks to the pool
    void checkDeps(); // read new task from new tasks buffer and update their deps
    void schedule(); // Take ready tasks from global list and send them to a core
    void handleFinished();

    int checkDeps(task&);
    void send_task_core(task&);
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
    static sc_fifo<task>* creator_rdy(const char* name, size_t i) {
      return new sc_fifo<task>("Ready Tasks", NEXUS2_RDY_TASKS_NUM);
    }
    static sc_fifo<task>* creator_fin(const char* name, size_t i) {
      return new sc_fifo<task>("Finished Tasks", NEXUS2_FIN_TASKS_NUM);
    }
    SC_CTOR(nexus) : tds_buffer("TDs Buffer", NEXUS2_TDS_BUFFER_DEPTH), new_tasks("New Tasks", NEXUS2_NEW_TASKS_NUM),
      global_ready_tasks("Global Ready Tasks", NEXUS2_GLOBAL_TASK_NUM){
      t_in_f.initialize(false);
      previous_task.id = 0;
      rdy.initialize(true);
      currentCore = 0;
      ci_ready_tasks.init(CORE_NUM, creator_rdy);
      ci_finished_tasks.init(CORE_NUM, creator_fin);
      t_ins.init(CORE_NUM);
      t_in_vs.init(CORE_NUM);
      t_in_fs.init(CORE_NUM);
      t_outs.init(CORE_NUM);
      t_out_vs.init(CORE_NUM);
      t_out_fs.init(CORE_NUM);
      rdys.init(CORE_NUM);
      for (int i = 0; i < CORE_NUM; i++) {
        t_in_vs[i].initialize(false);
        t_out_fs[i].initialize(false);
      }
      t_out_v.initialize(false);
      t_ready.initialize(false);
      /*
      #ifdef DEBUG
      SC_CTHREAD(debug_thread, clk.pos());
      #endif
      previous_f_task.id = 0;
*/
      task_pool = new TaskPool(NEXUS2_TASK_NUM);
      deps_table = new DependenceTable(NEXUS2_DEPS_TABLE_SIZE);
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
      SC_CTHREAD(checkDeps, clk.pos());
      SC_CTHREAD(schedule, clk.pos());
      SC_CTHREAD(handleFinished, clk.pos());
    }
  };
}
#endif
