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
      task_pool->print_entries();
    }
    else if (d == 4) {
      // Print the task table
      PRINTL("DEBUG: task_table:","");
      task_table->print_entries();
    }
    else if (d == 5) {
      // Print ConsumersTable
      PRINTL("DEBUG: consumers_table:","");
      consumers_table->print_entries();
    }
    else if (d == 6) {
      // Print ProducersTable
      PRINTL("DEBUG: producers_table:","");
      producers_table->print_entries();
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
          wait();
        }
        PRINTL("Nexus1 : Received new task %d", t.id);
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
    task *t = new task;
    while (!in_buffer.nb_read(*t)) {
      wait();
    }
    PRINTL("Loading this task %d", t->id);
    TaskPoolEntry *tpe = new TaskPoolEntry(*t);
    while(!task_pool->add_entry(tpe, t->id)) {
      wait();
    }
    PRINTL("Task %d was added to task pool", t->id);
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
    PRINTL("Sending ready task %d", t.id);
    t_ready_out_v.write(true);
    t_ready_out.write(t);
    do {
      wait();
    }while(!t_ready_out_f.read());
    t_ready_out_v.write(false);
    wait();
  }
}

void nexus::add_to_task_table(task* t) {
  TaskTableEntry* tte = new TaskTableEntry(*t);
  int deps = calculate_deps(t);
  tte->set_deps(deps);
  PRINTL("Task %d has %d deps count", t->id, tte->get_deps());
  while(!task_table->add_entry(tte, t->id)) {
    wait();
  }
  PRINTL("Task %d added to table", t->id);
  if (deps == 0) {
    PRINTL("adding task %d to queue", t->id);
    while (!task_queue.nb_write(*t)) {
      wait();
    }
  }
}

int nexus::add_input_prod(mem_addr input, task *t) {
  ProducersTableEntry* pr = producers_table->get_entry_for_addr(input);
  if (pr != nullptr) {
    while(!pr->add_task(*t)) {
      wait();
    }
    return 1;
  }
  return 0;
}

int nexus::add_input_cons(mem_addr input, task *t) {
  ProducersTableEntry *pte = producers_table->get_entry_for_addr(input);
  if (pte == nullptr) {
    ConsumersTableEntry* ct = consumers_table->get_entry_for_addr(input);
    if (ct != nullptr) {
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
    if (ct != nullptr) {
      while(!ct->add_task(*t)) {
        wait();
      }
    }
  }

  return 0;
}

int nexus::add_output_prod(mem_addr output, task *t) {
  ProducersTableEntry* pt = producers_table->get_entry_for_addr(output);
  if (pt != nullptr) {
    while(!pt->add_task(*t)) {
      wait();
    }
    return 1;
  }
  while(!producers_table->add_task(output, *t)) {
    wait();
  }
  return 0;
}

int nexus::add_output_cons(mem_addr output, task *t) {
  ConsumersTableEntry* ct = consumers_table->get_entry_for_addr(output);
  if (ct != nullptr) {
    while(!ct->add_task(*t)) {
      wait();
    }
    return 1;
  }
  return 0;
}

int nexus::calculate_deps(task* t) {
  // Here we fill data in producers and consumers tables
  int deps = 0;
  //PRINTL("Adding this task %d to tables", t->id);

  for (int i = 0; i < t->input_args; i++) {
    // Process each input arg
    deps += add_input_prod(t->get_input_arg(i), t);
    deps += add_input_cons(t->get_input_arg(i), t);
  }

  for (int i = 0; i < t->output_args; i++) {
    // Process each output arg
    int outputDepsCons = add_output_cons(t->get_output_arg(i), t);
    int outputDepsProd = add_output_prod(t->get_output_arg(i), t);
    if (outputDepsCons == 0) {
      deps += outputDepsProd;
    }
    else {
      deps += outputDepsCons;
    }
  }
  return deps;
}

/*void nexus::send_task() {
  t_ready.write(false);
  while(true) {
    task t;
    if (task_queue.nb_read(t)) {
      PRINTL("Can send task %d out", t.id);
      t_ready.write(true);

      while(t_out_f.read() == false) {
        wait();
      }
      t_out.write(t);
      t_out_v.write(true);
      wait();
      while (t_out_f.read() == false) {
        wait();
      }
      //task_table->set_sent(t);
      t_out_v.write(false);
    }
    else {
      t_ready.write(false);
    }
    wait();
  }
}*/

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
  PRINTL("Checking output %d", addr);
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
      PRINTL("Decrease number of deps for task %d", t.id);
      for (int i = 1; i < pte->size(); i++) {
        t = pte->get_task(i);
        if (check_task_input(t, addr)) {
          task_table->dec_deps(t.id);
          PRINTL("Decrease number of deps for task %d", t.id);
        }
      }
    }
  }
}

void nexus::check_input(mem_addr addr) {
  PRINTL("Checking input %d", addr);
  task t;
  ConsumersTableEntry *cte = consumers_table->get_entry_for_addr(addr);
  if (cte && cte->get_deps() > 0) {
    cte->dec_deps();
    if (cte->get_deps() == 0 && !cte->empty()) {
      t = cte->get_task(0);
      task_table->dec_deps(t.id);
      PRINTL("Decrease number of deps for task %d", t.id);
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
      PRINTL("Decrease number of deps for task %d", t.id);
    }
  }
}

void nexus::delete_task(task *t) {
  task t_to_delete = task_table->get_task(t->id);
  PRINTL("Deleteing task %d", t_to_delete.id);
  producers_table->delete_task(t->id);
  consumers_table->delete_task(t->id);
  for (int i = 0; i < t->output_args; i++) {
    check_output(t->get_output_arg(i));
  }
  for (int i = 0; i < t->input_args; i++) {
    check_input(t->get_input_arg(i));
  }
  task_table->delete_task(t->id);
}

void nexus::schedule_tasks() {
  // Here I loop through all tasks and send ready ones
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
        PRINTL("Finished task %d", t.id);
        this->delete_task(&t);
        this->schedule_tasks();
        t_f_in_f.write(true);
        wait();
        // Now delete task from all tables
        //delete_task(t);
      }
    }
    wait();
  }
}
