#include <nexus1.h>
#include <task.h>
#include <utils.h>

using namespace nexus1;

#ifdef DEBUG
void nexus::debug_print(int d) {
  if (d != 0) {
    if (d == 1) {
      // Print in_buffer
      PRINTL("DEBUG: in_buffer:","");
      in_buffer.dump();
    }
    else if (d == 2) {
      // Print the task queue
      PRINTL("DEBUG: task_queue:","");
      task_queue.dump();
    }
    else if (d == 3) {
      // Print the task pool
      PRINTL("DEBUG: task_pool:","");
      cout << task_pool << endl;
    }
    else if (d == 4) {
      // Print the task table
      PRINTL("DEBUG: task_table:","");
      cout << task_table << endl;
    }
    else if (d == 5) {
      // Print ConsumersTable
      PRINTL("DEBUG: consumers_table:","");
      cout << consumers_table << endl;
    }
    else if (d == 6) {
      // Print ProducersTable
      PRINTL("DEBUG: producers_table:","");
      cout << producers_table << endl;
    }
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

// Implement nexus1 thread functions

// Read tasks from the board unit and store them in the in_buffer
void nexus::receive() {
  rdy.write(true);
  t_in_f.write(false);

  while (true) {
    t_in_f.write(false);
    if (t_in_v.read() == true) {
      task t = t_in.read();
      if (t != previous_task) {
        previous_task = t;
        // Now the new task is read just write it to in_buffer
        while(!in_buffer.nb_write(t)) {
          rdy.write(false);
          //PRINTL("In buffer is full", "");
          wait();
        }
        PRINTL("nexus::receive Received new task %d", t.id);
        rdy.write(true);
        t_in_f.write(true);
      }
    }
    wait();
  }
}

// Take tasks from the in_buffer and write them to tables (Task, Producers and Consumers Tables)
void nexus::load() {
  while(true) {
    task t = task();
    while (!in_buffer.nb_read(t)) {
      wait();
    }
    PRINTL("nexus::load: Adding task %d to task pool", t.id);
    while(!task_pool->add_entry(TaskPoolEntry(t))) {
      wait();
    }
    PRINTL("nexus::load: Task %d was added to task pool", t.id);
    add_to_task_table(t);
    wait();
  }
}

void nexus::send_ready_task() {
  task t;
  while(true) {
    while (!task_queue.nb_read(t)) {
      wait();
    }
    PRINTL("nexus::send_ready_task: Sending ready task %d to board", t.id);
    t_ready_out_v.write(true);
    t_ready_out.write(t);
    do {
      wait();
    }while(!t_ready_out_f.read());
    t_ready_out_v.write(false);
    task_table->set_task_sent(t);
    PRINTL("nexus::send_ready_task: Task %d was sent to board", t.id);
    wait();
  }
}

void nexus::add_to_task_table(task& t) {
  TaskTableEntry tte = TaskTableEntry(t);
  PRINTL("nexus::add_to_task_table: calculate deps for task %d", t.id);
  while(!task_table->has_empty_entries()) {
    wait();
  }
  task_table_mutex.lock();
  int deps = calculate_deps(t);
  tte.set_deps(deps);
  task_table_mutex.unlock();
  PRINTL("nexus::add_to_task_table: Task %d has %d deps count", t.id, tte.get_deps());
  while(!task_table->add_entry(tte)) {
    wait();
  }
  PRINTL("nexus::add_to_task_table: Task %d added to table", t.id);
  if (deps == 0) {
    PRINTL("nexus::add_to_task_table: adding task %d to task queue", t.id);
    while (!task_queue.nb_write(t)) {
      wait();
    }
    PRINTL("nexus::add_to_task_table: Task %d was added to task queue", t.id);
  }
}

int nexus::add_input_prod(mem_addr input, task& t) {
  ProducersTableEntry* pr = producers_table->get_entry_for_addr(input);
  wait();
  if (pr != nullptr) {
    for (int i = 0; i < pr->size(); i++) {
      if (!check_task_input(pr->get_task(i), input)) {
        wait();
        while(!pr->add_task(t)) {
          wait();
        }
        return 1;
      }
    }
  }
  return 0;
}

int nexus::add_input_cons(mem_addr input, task& t) {
  ProducersTableEntry *pte = producers_table->get_entry_for_addr(input);
  wait();
  if (pte == nullptr) {
    wait();
    ConsumersTableEntry* ct = consumers_table->get_entry_for_addr(input);
    wait();
    if (ct != nullptr) {
      wait();
      ct->inc_deps();
    }
    else {
      while(!consumers_table->add_addr(input)) {
        wait();
      }
    }
  }
  else {
    ConsumersTableEntry* ct = consumers_table->get_entry_for_addr(input);
    wait();
    if (ct != nullptr) {
      wait();
      while(!ct->add_task(t)) {
        wait();
      }
    }
  }

  return 0;
}

int nexus::add_output_prod(mem_addr output, task& t) {
  ProducersTableEntry* pt = producers_table->get_entry_for_addr(output);
  wait();
  if (pt != nullptr) {
    wait();
    while(!pt->add_task(t)) {
      wait();
    }
    return 1;
  }
  while(!producers_table->add_task(output, t)) {
    wait();
  }
  return 0;
}

int nexus::add_output_cons(mem_addr output, task& t) {
  ConsumersTableEntry* ct = consumers_table->get_entry_for_addr(output);
  wait();
  if (ct != nullptr) {
    wait();
    while(!ct->add_task(t)) {
      wait();
    }
    return 1;
  }
  return 0;
}

int nexus::calculate_deps(task& t) {
  // Here we fill data in producers and consumers tables
  int deps = 0;

  for (int i = 0; i < t.input_args; i++) {
    // Process each input arg
    deps += add_input_prod(t.get_input_arg(i), t);
    deps += add_input_cons(t.get_input_arg(i), t);
    PRINTL("nexus::calculate_deps: deps for input %d task %d are %d", t.get_input_arg(i), t.id, deps);
  }

  for (int i = 0; i < t.output_args; i++) {
    // Process each output arg
    int outputDepsCons = add_output_cons(t.get_output_arg(i), t);
    int outputDepsProd = add_output_prod(t.get_output_arg(i), t);
    if (outputDepsCons == 0) {
      deps += outputDepsProd;
      PRINTL("nexus::calculate_deps: deps for output %d task %d are %d", t.get_output_arg(i), t.id, outputDepsProd);
    }
    else {
      deps += outputDepsCons;
      PRINTL("nexus::calculate_deps: deps for output %d task %d are %d", t.get_output_arg(i), t.id, outputDepsCons);
    }
  }
  return deps;
}

bool nexus::check_task_input(task t, mem_addr addr) {
  for (int i = 0; i < t.input_args; i++) {
    if(t.get_input_arg(i) == addr) {
      return true;
    }
  }
  return false;
}

bool nexus::check_task_output(task t, mem_addr addr) {
  for (int i = 0; i < t.output_args; i++) {
    if(t.get_output_arg(i) == addr) {
      return true;
    }
  }
  return false;
}

void nexus::check_output(mem_addr addr) {
  ConsumersTableEntry *cte = consumers_table->get_entry_for_addr(addr);
  if (cte && !cte->empty() && cte->get_deps() > 0) {
    cte->dec_deps();
  }
  else {
    ProducersTableEntry *pte = producers_table->get_entry_for_addr(addr);
    if (pte) {
      if (pte->empty()) {
        return;
      }
      task t = pte->get_task(0);
      task_table->dec_deps(t.id);
      if (check_task_input(t, addr)) {
        for (int i = 1; i < pte->size(); i++) {
          t = pte->get_task(i);
          if (check_task_input(t, addr)) {
            task_table->dec_deps(t.id);
          }
        }
      }
    }
  }
}

void nexus::check_input(mem_addr addr) {
  task t;
  ConsumersTableEntry *cte = consumers_table->get_entry_for_addr(addr);
  if (cte && cte->get_deps() > 0) {
    cte->dec_deps();
    if (cte->get_deps() == 0 && !cte->empty()) {
      t = cte->get_task(0);
      task_table->dec_deps(t.id);
    }
  }
  else {
    ProducersTableEntry *pte = producers_table->get_entry_for_addr(addr);
    if (pte) {
      if (pte->empty()) {
        return;
      }
      if (check_task_input(pte->get_task(0), addr)) {
        return;
      }
      t = pte->get_task(0);
      task_table->dec_deps(t.id);
    }
  }
}

void nexus::delete_task(task *t) {
  task_table_mutex.lock();
  task t_to_delete = task_table->get_task(t->id);
  PRINTL("nexus::delete_task: Deleteing task %d", t_to_delete.id);
  producers_table->delete_task(t->id);
  wait();
  consumers_table->delete_task(t->id);
  wait();
  for (int i = 0; i < t->output_args; i++) {
    check_output(t->get_output_arg(i));
    wait();
  }
  for (int i = 0; i < t->input_args; i++) {
    check_input(t->get_input_arg(i));
    wait();
  }
  task_table->delete_task(t->id);
  wait();
  task_pool->delete_task(t->id);
  wait();
  task_table_mutex.unlock();
  PRINTL("nexus::delete_task: Task %d was deleted", t_to_delete.id);
}

void nexus::schedule_tasks() {
  // Here I loop through all tasks and send ready ones
  for (auto it = task_table->begin(); it != task_table->end(); it++) {
    if (it->get_deps() == 0 && it->get_status() != TaskStatus::SENT) {
      while(!task_queue.nb_write(it->get_task())) {
        wait();
      }
    }
  }
  /*for (int i = 0; i < task_table->size(); i++) {
    auto tte = task_table->get_entry_by_index(i);
    if (tte) {
      if (tte->get_deps() == 0 && tte->get_status() != TaskStatus::SENT) {
        while(!task_queue.nb_write(tte->get_task())) {
          wait();
        }
      }
    }
  }*/
}

void nexus::read_finished() {
  t_f_in_f.write(false);

  while (true) {
    t_f_in_f.write(false);
    if (t_f_in_v.read() == true) {
      task t = t_f_in.read();
      t_f_in_f.write(false);
      if (t != previous_f_task) {
        previous_f_task = t;
        PRINTL("nexus::read_finished: Finished task %d", t.id);
        this->delete_task(&t);
        this->schedule_tasks();
        t_f_in_f.write(true);
        wait();
      }
    }
    wait();
  }
}
