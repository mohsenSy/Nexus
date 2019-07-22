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

void nexus::add_to_task_table(task* t) {
  TaskTableEntry* tte = new TaskTableEntry(*t);
  int deps = calculate_deps(t);
  tte->set_deps(deps);
  PRINTL("Task %d has %d deps count", t->id, tte->get_deps());
  while(!task_table->add_entry(tte, t->id)) {
    wait();
  }
  if (deps == 0) {
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
    deps += add_output_cons(t->get_output_arg(i), t);
    deps += add_output_prod(t->get_output_arg(i), t);
  }
  return deps;
}

void nexus::send_task() {
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
}

/*
void nexus::schedule() {
  while (true) {
    TaskTableEntry* tte = task_table->get_ready_task();
    if ( tte != NULL) {
      PRINTL("Can send this task %d to execution", tte->t->id);
      send_task(tte);
    }
    wait();
  }
}
*/

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
        task_table->delete_task(t.id);
        producers_table->delete_task(t.id);
        consumers_table->delete_task(t.id);
        t_f_in_f.write(true);
        wait();
        // Now delete task from all tables
        //delete_task(t);
      }
    }
    wait();
  }
}
/*
// Implement other nexus1 methods

int nexus::calculate_deps(task* t) {
  // Here we fill data in producers and consumers tables
  int deps = 0;

  for (int i = 0; i < t->output_args; i++) {
    // Process each output arg
    cons_table_entry* p = (cons_table_entry *)consumers_table->get_entry(t->get_output_arg(i));
    if (p != NULL) {
      consumers_table->add_to_kick_off_list(p->addr, *t);
      deps += 1;
      PRINTL("Added to kick off list of addr %d, task %d in consumers", p->addr, t->id);
    }
    prod_table_entry* pr = (prod_table_entry *)producers_table->get_entry(t->get_output_arg(i));
    if (pr == NULL) {
      pr = new prod_table_entry;
      pr->index = 0;
      pr->addr = t->get_output_arg(i);
      PRINTL("Added new addr %d to task %d in producers", pr->addr, t->id);
      while (!producers_table->add_entry(pr)) {
        wait();
      }
    }
    else {
      producers_table->add_to_kick_off_list(pr->addr, *t);
      deps += 1;
      PRINTL("Added to kick off list of addr %d, task %d in producers", pr->addr, t->id);
    }
  }

  for (int i = 0; i < t->input_args; i++) {
    // Process each input arg
    prod_table_entry* pr = (prod_table_entry *)producers_table->get_entry(t->get_input_arg(i));
    if (pr != NULL) {
      producers_table->add_to_kick_off_list(pr->addr, *t);
      deps += 1;
      PRINTL("Added to kick off list of addr %d, task %d in producers", pr->addr, t->id);
    }
    cons_table_entry* p = (cons_table_entry *)consumers_table->get_entry(t->get_input_arg(i));
    if (p == NULL) {
      p = new cons_table_entry;
      p->num_of_deps = 1;
      p->index = 0;
      p->addr = t->get_input_arg(i);
      PRINTL("Added new addr %d to task %d in consumers", p->addr, t->id);
      while(!consumers_table->add_entry(p)) {
        wait();
      }
    }
    else {
      if (p->index != 0) {
        // There are tasks waiting to write to this memory location
        // Add the task to the kick off list
        consumers_table->add_to_kick_off_list(p->addr, *t);
        deps += 1;
        PRINTL("Added to kick of list of addr %d, task %d in consumers", p->addr, t->id);
      }
      else {
        // Another task wants to read from this memory location
        consumers_table->increment_deps(p->addr);
        PRINTL("Incremented number of deps for addr %d in task %d", p->addr, t->id);
      }
    }
  }

  return deps;
}
*/

/*


void nexus::delete_task(task& t) {
  PRINTL("Deleteing task %d from all tables", t.id);
  task_table->delete_entry(t.id);
  task_pool->delete_entry(t.id);
  for (int i = 0; i < t.input_args; i++) {
    consumers_table->delete_addr(t.get_input_arg(i));
  }
  //producers_table->print_entries();
}*/
