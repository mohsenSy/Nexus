#include <nexus2.h>
#include <utils.h>
using namespace nexus2;

void nexus::getTDs() {
  rdy.write(true);
  while (true) {
    t_in_f.write(false);
    rdy.write(true);
    if (t_in_v.read()) {
      task t = t_in.read();
      if (previous_task != t) {
        previous_task = t;
        while(!tds_buffer.nb_write(t)) {
          rdy.write(false);
          wait();
        }
        PRINTL("received task %d", t.id);
        t_in_f.write(true);
        rdy.write(true);
        wait();
      }
    }
    wait();
  }
}

void nexus::writeTP() {
  while(true) {
    task t;
    while(!tds_buffer.nb_read(t)) {
      wait();
    }
    PRINTL("Writing task %d to task pool", t.id);
    while(!task_pool->addTask(t)) {
      wait();
    }
    while(!new_tasks.nb_write(t)) {
      wait();
    }
    PRINTL("Task %d was added to the pool", t.id);
    wait();
  }
}
