#ifndef _NEXUS1__H
#define _NEXUS1__H
#include <table.h>
#include <types.h>
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
            std::cout << "Kick Of List: ";
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

  class ConsumersTableEntry {
    private:
      mem_addr addr;
      int deps;
      KickOfList kick_of_list;
    public:
      ConsumersTableEntry(mem_addr addr) {
        this->addr = addr;
        this->deps = 1;
        this->kick_of_list = KickOfList();
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

      void del_task() {
        kick_of_list.pop();
      }

      void print() {
        std::cout << "addr: " << addr << " deps: " << deps << std::endl;
        kick_of_list.print();
      }
  };
  enum TaskStatus { NEW, SENT, FINISHED };
  std::string status_array[] = {"NEW", "SENT", "FINISHED"};
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
      void inc_deps(int i = 1) {
        this->deps += i;
      }
      bool is_ready() {
        return this->deps == 0;
      }
      void print() {
        std::cout << "task id: " << t.id << " status: " << status_array[this->status] << " deps: " << this->deps << std::endl;
      }
  };
};
#endif
