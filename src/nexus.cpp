#include <nexus.h>
#include <task.h>
#include <utils.h>

void nexus1::receive() {
  rdy.write(true);
  t_in_f.write(false);

  while (true) {
    t_in_f.write(false);
    if (t_in_v.read() == true) {
      task t = t_in.read();
      if (t != previous_task) {
        previous_task = t;
        PRINTL("Received new task %d", t.id);
        // Now the new task is read just write it to in_buffer
        while(!in_buffer.nb_write(t)) {
          rdy.write(false);
          wait();
        }
        rdy.write(true);
        t_in_f.write(true);
      }

    }
    wait();

  }

}

void nexus1::load() {
  while(true) {
    task *t = new task;
    while (!in_buffer.nb_read(*t)) {
      wait();
    }
    PRINTL("Loading this task %d", t->id);
    while(!task_pool->add_entry((void *)t, t->id)) {
      //PRINTL("Task %d waiting for task pool", t->id);
      wait();
    }
    add_to_task_table(t);
    wait();
    //task_table->print_entries();
  }


}

void nexus1::add_to_task_table(task* t) {
  TaskTableEntry* tte = new TaskTableEntry;
  tte->id = t->id;
  tte->t = t;
  tte->status = NEW;
  tte->num_of_deps = calculate_deps(t);
  PRINTL("Task %d has %d deps count", t->id, tte->num_of_deps);
  while(!task_table->add_entry((void *)tte, tte->id)) {
    //PRINTL("Task %d waiting for task table", tte->id);
    wait();
  }
}

int nexus1::calculate_deps(task* t) {
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

void nexus1::schedule() {
  while (true) {
    TaskTableEntry* tte = task_table->get_ready_task();
    if ( tte != NULL) {
      PRINTL("Can send this task %d to execution", tte->t->id);
      send_task(tte);
    }
    wait();
  }
}

void nexus1::send_task(TaskTableEntry *t) {
    // Make sure t_out_f is true
    while (t_out_f.read() == false) {
      PRINTL("Waiting for other unit to finish reading", "");
      wait();
    }
    t_out.write(*t->t);
    t_out_v.write(true);
    wait();
    while (t_out_f.read() == false) {
      PRINTL("Waiting","");
      wait();
    }
    /*do {
      //PRINTL("Wait", "");
      std::cout << t_out_f.read();
      wait();
    } while(t_out_f.read() == false);*/
    t->status = SENT;
    PRINTL("Sent task %d", t->t->id);
    t_out_v.write(false);
}

void nexus1::read_finished() {
  t_f_in_f.write(false);

  while (true) {
    t_f_in_f.write(false);
    if (t_f_in_v.read() == true) {
      task t = t_f_in.read();
      t_f_in_f.write(false);
      if (t != previous_f_task) {
        previous_f_task = t;
        PRINTL("Finished task %d", t.id);
        t_f_in_f.write(true);
        wait();
        // Now delete task from all tables
        delete_task(t);
      }
    }
    wait();
  }
}

void nexus1::delete_task(task& t) {
  PRINTL("Deleteing task %d from all tables", t.id);
  task_table->delete_entry(t.id);
  task_pool->delete_entry(t.id);
  for (int i = 0; i < t.input_args; i++) {
    consumers_table->delete_addr(t.get_input_arg(i));
  }
  //producers_table->print_entries();
}
