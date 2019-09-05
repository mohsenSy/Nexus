#include <nexus2.h>
#include <utils.h>
using namespace nexus2;

#ifdef DEBUG
void nexus::debug_print(int d) {
  if (d == 1) {
    task_pool->print_entries();
  }
  else if (d == 2) {
    deps_table->print_entries();
  }
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
        wait();
        while(!tds_buffer.nb_write(t)) {
          rdy.write(false);
          print_once(2, "Nexus TDs buffer is full cannot receive new task %d", t.id);
          wait();
        }
        reset_printe_once(2);
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
    do {
      wait();
    } while(!tds_buffer.nb_read(t));
    PRINTL("Writing task %d to task pool", t.id);
    while(!task_pool->addTask(t)) {
      print_once(3, "Cannot add new task %d to nexus task pool", t.id);
      wait();
    }
    reset_printe_once(3);
    do {
      print_once(4, "Cannot add new task %d", t.id);
      PRINTL("Cannot add new task %d", t.id);
      wait();
    } while(!new_tasks.nb_write(t));
    reset_printe_once(4);
    PRINTL("Task %d was added to the pool", t.id);
    wait();
  }
}

void nexus::checkDeps() {
  while (true) {
    task t;
    do {
      wait();
    } while(!new_tasks.nb_read(t));
    deps_table_mutex.lock();
    wait();
    int deps = checkDeps(t);
    task_pool->setDc(t, deps);
    deps_table_mutex.unlock();
    DependenceTableEntry* dte = deps_table->get_data(0);
    wait();
    deps_table->set_data(0, dte);
    PRINTL("Deps for task %d are %d", t.id, deps);
    if (deps == 0) {
      do {
        print_once(5, "Cannot add task %d to global ready queue", t.id);
        wait();
      } while(!global_ready_tasks.nb_write(t));
      reset_printe_once(5);
    }
    wait();
  }
}


void nexus::schedule() {
  while (true) {
    //global_ready_tasks.dump();
    task t;
    do {
      wait();
    } while(!global_ready_tasks.nb_read(t));
    send_task_core(t);
    wait();
  }
}

void nexus::send_task_core(task &t) {
  if (task_pool->getSent(t)) {
    return;
  }
  core_mutex.lock();
  wait();
  while(!rdys[currentCore]) {
    wait();
    print_once(6, "Waiting for a ready core to send task %d", t.id);
    currentCore = ++currentCore % CORE_NUM;
  }
  reset_printe_once(6);
  PRINTL("Sending task %d to core %d", t.id, currentCore);
  t_ins[currentCore].write(t);
  t_in_vs[currentCore].write(true);
  do {
    print_once(7, "Waiting for core %d to read task %d", currentCore, t.id);
    wait();
  }while(!t_in_fs[currentCore].read());
  reset_printe_once(7);
  LOG(name(), "Sent task %d to core %d", t.id, currentCore);
  t_in_vs[currentCore].write(false);
  if (first) {
    first = false;
  }
  else {
    first = true;
    currentCore = ++currentCore % CORE_NUM;
  }
  deps_table_mutex.lock();
  wait();
  task_pool->setSent(t, true);
  deps_table_mutex.unlock();
  core_mutex.unlock();
  wait();
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
  deps_table_mutex.lock();
  wait();
  for (int i = 0; i < t.input_args; i++) {
    mem_addr input = t.get_input_arg(i);
    if (!check_task_output(t, input)) {
      int rdrs = deps_table->decRdrs(input);
      wait();
      PRINTL("Decrement deps for address %d", input);
      if (rdrs == 0 && !deps_table->getWw(input)) {
        deps_table->deleteAddr(input);
        wait();
      }
      else if (rdrs == 0 && deps_table->getWw(input)) {
        task t = deps_table->pop(input);
        wait();
        task_pool->decDeps(t);
        PRINTL("Decrement deps for task %d", t.id);
        wait();
      }
    }
  }
  for (int i = 0; i < t.output_args; i++) {
    mem_addr output = t.get_output_arg(i);
    int size = deps_table->getListSize(output);
    if (size == 0) {
      deps_table->deleteAddr(output);
      wait();
    }
    else {
      while(deps_table->getListSize(output) > 0) {
        PRINTL("Processing output %d", output);
        //deps_table->print_entries();
        task t = deps_table->pop(output);
        wait();
        //deps_table->print_entries();
        task_pool->decDeps(t);
        wait();
        PRINTL("Decrease deps for task %d", t.id);
        if (check_task_output(t, output)) {
          break;
        }
        deps_table->incRdrs(output);
        wait();
      }
    }
  }
  task_pool->deleteTask(t);
  deps_table_mutex.unlock();
  PRINTL("Deleted task %d", t.id);
  wait();
}

void nexus::scheduleReadyTasks() {
  for (int i = 0; i < NEXUS2_TASK_NUM; i++) {
    TaskPoolEntry* tpe = task_pool->getEntryByIndex(i);
    if (tpe) {
      wait();
      if (tpe->getDc() == 0 && !tpe->getSent() && tpe->getDepsReady()) {
        wait();
        if (!global_ready_tasks.nb_write(tpe->getTask())) {
          return;
        }
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
        LOG(name(), "Finished task %d from core %d", t.id, i);
        t_out_fs[i].write(true);
        wait();
        t_out_fs[i].write(false);
        do {
          print_once(8, "Cannot add finished task %d to queue", t.id);
          wait();
        }while(!finished_tasks.nb_write(t));
        reset_printe_once(8);
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
    wait();
    if (!dte) {
      deps_table->addAddr(input);
      wait();
    }
    else {
      if (!dte->getIsOut() && !dte->getWw()) {
        dte->incRdrs();
        wait();
      }
      else {
        dte->addTask(t);
        wait();
        d++;
      }
    }
  }
  for (int i = 0 ; i < t.output_args; i++) {
    mem_addr output = t.get_output_arg(i);
    PRINTL("Processing output %d in task %d", output, t.id);
    DependenceTableEntry *dte = deps_table->getEntry(output);
    wait();
    if (!dte) {
      deps_table->addAddr(output, true);
      wait();
    }
    else {
      dte->addTask(t);
      wait();
      d++;
      if (!dte->getIsOut()) {
        dte->setWw(true);
        wait();
      }
    }
  }
  return d;
}
