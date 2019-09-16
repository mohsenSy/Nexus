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
      friend ostream& operator <<(ostream& out, const KickOfList& l) {
        for(int i = 0; i < l.tasks.size(); i++) {
          if (i == 0) {
            out << "Kick Of List:" << endl;
          }
          out << "i: " << i << " task: " << l.tasks[i].id << endl;
        }
        return out;
      }
  };

  class ProducersTableEntry {
    private:
      mem_addr addr;
      KickOfList kick_of_list;
      sc_mutex* m;
    public:
      ProducersTableEntry(mem_addr addr) {
        this->addr = addr;
        kick_of_list = KickOfList();
        //kick_of_list.push(task());
         m = new sc_mutex();
      }
      ProducersTableEntry(mem_addr addr, task t) {
        this->addr = addr;
        kick_of_list = KickOfList();
        kick_of_list.push(t);
         m = new sc_mutex();
      }

      mem_addr get_addr() {
        return addr;
      }

      void set_addr(mem_addr addr) {
        m->lock();
        this->addr = addr;
        m->unlock();
      }

      bool add_task(task t) {
        return kick_of_list.push(t);
      }

      void del_task() {
        kick_of_list.pop();
      }

      bool empty() {
        return kick_of_list.empty();
      }

      task get_task(int index) {
        return *kick_of_list.get_task(index);
      }

      void delete_task(int id) {
        kick_of_list.delete_task(id);
      }

      int size() {
        return kick_of_list.get_size();
      }
      friend ostream& operator <<(ostream& out, const ProducersTableEntry& pte) {
        out << "addr: " << pte.addr << endl;
        out << "list: " << pte.kick_of_list;
        return out;
      }
      bool operator ==(const ProducersTableEntry& pte) {
        return addr == pte.addr;
      }
  };

  class ProducersTable : public Table<ProducersTableEntry> {
    private:
      sc_mutex* m;
    public:
      ProducersTable(int count) : Table<ProducersTableEntry>(count) { m = new sc_mutex();}
      ProducersTableEntry *get_entry_for_addr(mem_addr addr) {
        return find_entry(ProducersTableEntry(addr));
      }
      bool add_task(mem_addr addr, task t) {
        m->lock();
        ProducersTableEntry *pte = this->get_entry_for_addr(addr);
        if (pte != nullptr) {
          m->unlock();
          return pte->add_task(t);
        }
        m->unlock();
        return add_entry(ProducersTableEntry(addr, t));
      }
      void delete_task(int id) {
        m->lock();
        for (auto it = begin(); it != end(); it++) {
          it->delete_task(id);
          if (it->empty()) {
            delete_entry(*it);
          }
        }
        /*for (int i = 0; i < count; i++) {
          if (entries[i] != nullptr && entries[i]->get_used() && entries[i]->get_data()) {
            ProducersTableEntry* pte = entries[i]->get_data();
            pte->delete_task(id);
            if (pte->empty()) {
              mem_addr addr = pte->get_addr();
              int id = *(int *)&addr;
              delete_entry(id);
            }
          }
        }*/
        m->unlock();
      }
  };

  class ConsumersTableEntry {
    private:
      mem_addr addr;
      int deps;
      KickOfList kick_of_list;
      sc_mutex* m;
    public:
      ConsumersTableEntry(mem_addr addr) {
        this->addr = addr;
        this->deps = 1;
        this->kick_of_list = KickOfList();
         m = new sc_mutex();
      }
      ConsumersTableEntry(mem_addr addr, task t) {
        this->addr = addr;
        this->deps = 0;
        this->kick_of_list = KickOfList();
        this->kick_of_list.push(t);
        m = new sc_mutex();
      }

      mem_addr get_addr() {
        return addr;
      }

      void set_addr(mem_addr addr) {
        m->lock();
        this->addr = addr;
        m->unlock();
      }

      int get_deps() {
        return deps;
      }

      void inc_deps(int i = 1) {
        m->lock();
        this->deps += i;
        m->unlock();
      }
      void dec_deps(int i = 1) {
        m->lock();
        this->deps -= i;
        m->unlock();
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

      void delete_task(int id) {
        m->lock();
        if (this->deps == 0) {
          kick_of_list.delete_task(id);
        }
        m->unlock();
      }

      task get_task(int index) {
        return *kick_of_list.get_task(index);
      }

      friend ostream& operator << (ostream& out, const ConsumersTableEntry& ct) {
        out << "addr: " << ct.addr << " deps: " << ct.deps << endl;
        out << ct.kick_of_list << endl;
        return out;
      }

      bool operator ==(const ConsumersTableEntry& cte) {
        return addr == cte.addr;
      }
  };

  class ConsumersTable : public Table<ConsumersTableEntry> {
    private:
      sc_mutex* m;
    public:
      ConsumersTable(int count) : Table<ConsumersTableEntry>(count) { m = new sc_mutex();}
      ConsumersTableEntry *get_entry_for_addr(mem_addr addr) {
        return find_entry(ConsumersTableEntry(addr));
        /*for (int i = 0; i < count; i++) {
          if (entries[i] != nullptr && entries[i]->get_used() && entries[i]->get_data() && entries[i]->get_data()->get_addr() == addr) {
            m->unlock();
            return entries[i]->get_data();
          }
        }
        m->unlock();
        return nullptr;*/
      }
      bool add_task(mem_addr addr, task t) {
        m->lock();
        ConsumersTableEntry *cte = this->get_entry_for_addr(addr);
        if (cte != nullptr) {
          m->unlock();
          return cte->add_task(t);
        }
        //cte = new ConsumersTableEntry(addr, t);
        m->unlock();
        return add_entry(ConsumersTableEntry(addr, t));
      }
      bool add_addr(mem_addr addr) {
        m->lock();
        ConsumersTableEntry *cte = this->get_entry_for_addr(addr);
        if (cte != nullptr) {
          cte->inc_deps();
          m->unlock();
          return true;
        }
        //cte = new ConsumersTableEntry(addr);
        m->unlock();
        return add_entry(ConsumersTableEntry(addr));
      }

      void delete_task(int id) {
        m->lock();
        for (auto it = begin(); it != end(); it++) {
          it->delete_task(id);
          if (it->empty()) {
            delete_entry(ConsumersTableEntry(it->get_addr()));
          }
        }
        /*for (int i = 0; i < count; i++) {
          if (entries[i] != nullptr && entries[i]->get_used() && entries[i]->get_data()) {
            ConsumersTableEntry* cte = entries[i]->get_data();
            cte->delete_task(id);
            if (cte->get_deps() == 0 && cte->empty()) {
              mem_addr addr = cte->get_addr();
              int id = *(int *)&addr;
              delete_entry(id);
            }
          }
        }*/
        m->unlock();
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
      sc_mutex* m;
    public:
      TaskTableEntry(task t) {
        this->t = t;
        status = NEW;
        deps = 0;
        m = new sc_mutex();
      }
      TaskTableEntry(int id) {
        t.id = id;
        status = NEW;
        deps = 0;
        m = new sc_mutex();
      }
      TaskTableEntry() {
        t.id = 0;
        status = NEW;
        deps = 0;
        m = new sc_mutex();
      }
      task get_task() {
        return t;
      }
      void set_task(task t) {
        m->lock();
        this->t = t;
        m->unlock();
      }
      TaskStatus get_status() {
        return status;
      }
      void set_status(TaskStatus new_status) {
        m->lock();
        this->status = new_status;
        m->unlock();
      }
      int get_deps() {
        return this->deps;
      }
      void set_deps(int deps) {
        m->lock();
        this->deps = deps;
        m->unlock();
      }
      void dec_deps(int i = 1) {
        m->lock();
        this->deps -= i;
        m->unlock();
      }
      bool is_ready() {
        return this->deps == 0;
      }
      friend ostream& operator << (ostream& out, const TaskTableEntry& tte) {
        out << "task id: " << tte.t.id << " status: " << status_array[tte.status] << " deps: " << tte.deps << endl;
        return out;
      }
      bool operator ==(const TaskTableEntry& tte) {
        return tte.t.id == t.id;
      }
  };

  class TaskTable : public Table<TaskTableEntry> {
    private:
      sc_mutex* m;
    public:
      TaskTable(int count) : Table<TaskTableEntry>(count) { m = new sc_mutex();}
      task get_task(int id) {
        //m->lock();
        TaskTableEntry tte = Table::get_entry(TaskTableEntry(id));
        return tte.get_task();
        /*task t;
        t.id = 0;
        for (int i = 0; i < count; i++) {
          if (entries[i] != nullptr && entries[i]->get_used() && entries[i]->get_data() && entries[i]->get_data()->get_task().id == id) {
            m->unlock();
            return entries[i]->get_data()->get_task();
          }
        }
        m->unlock();
        return t;*/
      }
      /*TaskTableEntry* get_entry_by_index(int index) {
        if (index < 0 || index > count) {
          return nullptr;
        }
        if (entries[index] == nullptr) {
          return nullptr;
        }
        return entries[index]->get_data();
      }*/
      TaskTableEntry* get_entry(int id) {
        //m->lock();
        return find_entry(TaskTableEntry(id));
        /*for (int i = 0; i < count; i++) {
          if (entries[i] != nullptr && entries[i]->get_used() && entries[i]->get_data() && entries[i]->get_data()->get_task().id == id) {
            m->unlock();
            return entries[i]->get_data();
          }
        }
        m->unlock();
        return nullptr;*/
      }
      void dec_deps(int id) {
        //m->lock();
        find_entry(TaskTableEntry(id))->dec_deps();
        /*for (int i = 0; i < count; i++) {
          if (entries[i] != nullptr && entries[i]->get_used() && entries[i]->get_data() && entries[i]->get_data()->get_task().id == id) {
            entries[i]->get_data()->dec_deps();
          }
        }
        m->unlock();*/
      }
      void set_task_sent(task t) {
        m->lock();
        TaskTableEntry *tte = get_entry(t.id);
        tte->set_status(TaskStatus::SENT);
        m->unlock();
      }
      void delete_task(int id) {
        m->lock();
        delete_entry(TaskTableEntry(id));
        m->unlock();
      }
      int count() {
        return get_size();
      }
  };

  class TaskPoolEntry {
    private:
      task t;
      sc_mutex* m;
    public:
      TaskPoolEntry(task t) {
        this->t = t;
        m = new sc_mutex();
      }
      TaskPoolEntry(int id) {
        t.id = id;
        m = new sc_mutex();
      }
      task get_task() {
        return this->t;
      }
      void set_task(task t) {
        m->lock();
        this->t = t;
        m->unlock();
      }
      friend ostream& operator << (ostream& out, const TaskPoolEntry& tpe) {
        out << "task id: " << tpe.t.id << " inputs: " << tpe.t.input_args << " outputs: " << tpe.t.output_args << std::endl;
        return out;
      }
      bool operator == (const TaskPoolEntry& pte) {
        return t.id == pte.t.id;
      }
  };
  class TaskPool : public Table<TaskPoolEntry> {
    private:
      sc_mutex* m;
    public:
      TaskPool(int count) : Table<TaskPoolEntry>(count) { m = new sc_mutex();}
      void delete_task(int id) {
        m->lock();
        delete_entry(TaskPoolEntry(id));
        m->unlock();
      }
      int count() {
        return get_size();
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

    sc_fifo<task> in_buffer; // Buffer for received tasks.
    sc_fifo<task> task_queue; // Buffer for tasks ready for execution.

    TaskPool* task_pool;
    //Table<TaskTableEntry>* task_table;
    TaskTable* task_table;
    sc_mutex task_table_mutex;
    ProducersTable* producers_table;
    ConsumersTable* consumers_table;

    task previous_task;
    task previous_f_task;

    // Nexus1 threads
    void receive(); // Receive a new task and store it in receive buffer
    void load(); // Load a task from receive buffer to Task Pool and other tables
    void add_to_task_table(task&);
    int calculate_deps(task&);
    void send_task();

    int add_input_prod(mem_addr, task&);
    int add_input_cons(mem_addr, task&);
    int add_output_cons(mem_addr, task&);
    int add_output_prod(mem_addr, task&);
    void check_output(mem_addr);
    void check_input(mem_addr);
    bool check_task_input(task, mem_addr);
    bool check_task_output(task, mem_addr);
    void schedule_tasks();
    void send_ready_task();

    void read_finished(); // Read finished tasks and delete them.
    void delete_task(task*);

    SC_CTOR(nexus): in_buffer("in_buffer", NEXUS1_IN_BUFFER_DEPTH), task_queue("task_queue", NEXUS1_READY_QUEUE_DEPTH),  rdy("ready"), t_f_in_f("task_finished_input_valid"), t_f_in("task_finished_input"),t_f_in_v("task_finished_input_finished"),
      t_ready("task_ready"), t_out("task_output"), t_out_v("task_output_valid"), t_out_f("task_output_finished"), t_in_f("task_input_finished"), t_in_v("task_input_valid"),
      t_in("task_input"), clk("clock"){
      rdy.initialize(true);
      t_out_v.initialize(false);
      t_ready.initialize(false);
      #ifdef DEBUG
      SC_CTHREAD(debug_thread, clk.pos());
      #endif
      previous_task.id = 0;
      previous_f_task.id = 0;

      task_pool = new TaskPool(NEXUS1_TASK_NUM);
      task_table = new TaskTable(NEXUS1_TASK_TABLE_SIZE);
      producers_table = new ProducersTable(NEXUS1_PRODUCERS_TABLE_SIZE);
      consumers_table = new ConsumersTable(NEXUS1_CONSUMERS_TABLE_SIZE);

      PRINTL("new nexus 1 %s", name());
      SC_CTHREAD(receive, clk.pos());
      SC_CTHREAD(load, clk.pos());
      //SC_CTHREAD(send_task, clk.pos());
      //SC_CTHREAD(schedule, clk.pos());
      SC_CTHREAD(read_finished, clk.pos());
      SC_CTHREAD(send_ready_task, clk.pos());
    }
  };
};
#endif
