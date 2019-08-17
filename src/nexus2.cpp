#include <nexus2.h>
#include <utils.h>
using namespace nexus2;

#ifdef DEBUG
void nexus::debug_print(int d) {
}
void nexus::debug_thread() {
  while(true) {
    int d = debug.read();
    debug_print(d);
    wait();
  }
}
#endif

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

void nexus::checkDeps() {
  while (true) {
    task t;
    while(!new_tasks.nb_read(t)) {
      wait();
    }
    int deps = checkDeps(t);
    PRINTL("Deps for task %d are %d", t.id, deps);
    if (deps == 0) {
      while(!global_ready_tasks.nb_write(t)) {
        wait();
      }
    }
    wait();
  }
}


void nexus::schedule() {
  while (true) {
    task t;
    while(!global_ready_tasks.nb_read(t)) {
      wait();
    }
    PRINTL("Sending task %d to a core", t.id);
    send_task_core(t);
    wait();
  }
}

void nexus::send_task_core(task &t) {
  while(!rdys[currentCore]) {
    wait();
  }
  t_ins[currentCore].write(t);
  t_in_vs[currentCore].write(true);
  do {
    wait();
  }while(!t_in_fs[currentCore].read());
  t_in_vs[currentCore++].write(false);
  wait();
}

void nexus::handleFinished() {
  while(true) {
    for (int i = 0; i < CORE_NUM; i++) {
      if (t_out_vs[i].read()) {
        task t = t_outs[i].read();
        t_out_fs[i].write(true);
        PRINTL("Deleteing task %d from nexus tables", t.id);
        wait();
      }
      wait();
    }
    wait();
  }
}

int nexus::checkDeps(task &t) {
  int d = 0;
  PRINTL("Checking deps for %d", t.id);
  for (int i = 0 ; i < t.input_args; i++) {
    mem_addr input = t.get_input_arg(i);
    PRINTL("Processing input %d in task %d", input, t.id);
    DependenceTableEntry *dte = deps_table->getEntry(input);
    if (!dte) {
      deps_table->addAddr(input);
    }
    else {
      if (!dte->getIsOut() && !dte->getWw()) {
        dte->incRdrs();
      }
      else {
        dte->addTask(t);
        d++;
      }
    }
  }
  for (int i = 0 ; i < t.output_args; i++) {
    mem_addr output = t.get_output_arg(i);
    PRINTL("Processing output %d in task %d", output, t.id);
    DependenceTableEntry *dte = deps_table->getEntry(output);
    if (!dte) {
      deps_table->addAddr(output, true);
    }
    else {
      dte->addTask(t);
      d++;
      if (!dte->getIsOut()) {
        dte->setWw(true);
      }
    }
  }
  return d;
}
