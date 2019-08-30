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
    task_pool->setDc(t, deps);
    PRINTL("Deps for task %d are %d", t.id, deps);
    if (deps == 0) {
      while(!global_ready_tasks.nb_write(t)) {
        wait();
      }
      PRINTL("New global ready task %d", t.id);
    }
    wait();
  }
}


void nexus::schedule() {
  while (true) {
    //global_ready_tasks.dump();
    task t;
    while(!global_ready_tasks.nb_read(t)) {
      wait();
    }
    send_task_core(t);
    wait();
  }
}

void nexus::send_task_core(task &t) {
  if (task_pool->getSent(t)) {
    return;
  }
  core_mutex.lock();
  while(!rdys[currentCore]) {
    PRINTL("Waiting for core %d to be ready", currentCore);
    wait();
    currentCore = ++currentCore % CORE_NUM;
  }
  PRINTL("Sending task %d to core %d", t.id, currentCore);
  t_ins[currentCore].write(t);
  t_in_vs[currentCore].write(true);
  do {
    PRINTL("Waiting for core %d to read task", currentCore);
    wait();
  }while(!t_in_fs[currentCore].read());
  t_in_vs[currentCore].write(false);
  if (first) {
    first = false;
    PRINTL("First is %d", first);
  }
  else {
    first = true;
    currentCore = ++currentCore % CORE_NUM;
    PRINTL("New core %d", currentCore);
  }
  task_pool->setSent(t, true);
  wait();
  core_mutex.unlock();
}

bool nexus::check_task_input(task &t, mem_addr addr) {
  for (int i = 0; i < t.input_args; i++) {
    if(t.get_input_arg(i) == addr) {
      return true;
    }
  }
  return false;
}

bool nexus::check_task_output(task &t, mem_addr addr) {
  for (int i = 0; i < t.output_args; i++) {
    if(t.get_output_arg(i) == addr) {
      return true;
    }
  }
  return false;
}

void nexus::deleteTask(task& t) {
  PRINTL("Deleteing task %d from nexus tables", t.id);
  for (int i = 0; i < t.input_args; i++) {
    mem_addr input = t.get_input_arg(i);
    if (!check_task_output(t, input)) {
      int rdrs = deps_table->decRdrs(input);
      PRINTL("Decrement deps for address %d", input);
      if (rdrs == 0 && !deps_table->getWw(input)) {
        deps_table->deleteAddr(input);
      }
      else if (rdrs == 0 && deps_table->getWw(input)) {
        task t = deps_table->pop(input);
        task_pool->decDeps(t);
      }
    }
  }
  for (int i = 0; i < t.output_args; i++) {
    mem_addr output = t.get_output_arg(i);
    int size = deps_table->getListSize(output);
    if (size == 0) {
      deps_table->deleteAddr(output);
    }
    else {
      while(deps_table->getListSize(output) > 0) {
        PRINTL("Processing output %d", output);
        //deps_table->print_entries();
        task t = deps_table->pop(output);
        //deps_table->print_entries();
        task_pool->decDeps(t);
        PRINTL("Decrease deps for %d", t.id);
        if (check_task_output(t, output)) {
          break;
        }
        deps_table->decRdrs(output);
      }
    }
  }
  task_pool->deleteTask(t);
}

void nexus::scheduleReadyTasks() {
  for (int i = 0; i < NEXUS2_TASK_NUM; i++) {
    TaskPoolEntry* tpe = task_pool->getEntryByIndex(i);
    if (tpe) {
      if (tpe->getDc() == 0 && !tpe->getSent()) {
        global_ready_tasks.nb_write(tpe->getTask());
      }
    }
  }
}

void nexus::readFinished() {
  task t;
  while (true) {
    do {
      wait();
    }while(!finished_tasks.nb_read(t));
    deleteTask(t);
    scheduleReadyTasks();
    wait();
  }
}

void nexus::handleFinished() {
  while(true) {
    for (int i = 0; i < CORE_NUM; i++) {
      //PRINTL("Checking core %d", i);
      core_mutex.lock();
      if (t_out_vs[i].read()) {
        PRINTL("Reading finished task from core %d", i);
        task t = t_outs[i].read();
        t_out_fs[i].write(true);
        wait();
        t_out_fs[i].write(false);
        do {
          wait();
        }while(!finished_tasks.nb_write(t));
        wait();
      }
      core_mutex.unlock();
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