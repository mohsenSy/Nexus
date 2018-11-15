#include <nexus.h>
#include <task.h>
#include <utils.h>

bool TableEntry::is_used() {
  return used;
}

int TableEntry::get_id() {
  return id;
}

void* TableEntry::get_data() {
  return data;
}

void TableEntry::set_used(bool val) {
  used = val;
}

void TableEntry::set_id(int val) {
  id = val;
}

void TableEntry::set_data(void* d) {
  data = d;
}

bool Table::add_entry(void* en, int id) {
  // Adds en to the array of entries
  //std::cout << "In base class" << std::endl;
  // Find a valid entry
  for (int i = 0; i < count; i++) {
    //std::cout << i << std::endl;
    if (entries[i] == NULL) {
      // Add a new Entry
      entries[i] = new TableEntry(en, id);
      entries[i]->set_used(true);
      //std::cout << "Entry added new " << i << std::endl;
      return true;
    }
    if (!entries[i]->is_used()) {
      // The entry exists and is not used
      entries[i]->set_data(en);
      entries[i]->set_id(id);
      entries[i]->set_used(true);
      //std::cout << "Entry added old " << i << std::endl;
      return true;
    }
  }
  //std::cout << "No entry added" << std::endl;
  return false;
}

bool Table::delete_entry(int id) {
  for(int i = 0; i < count; i++) {
    if(entries[i] != NULL && entries[i]->is_used() && entries[i]->get_id() == id) {
      delete entries[i];
      return true;
    }
  }
  return false;
}

void* Table::get_entry(int id) {
  for (int i = 0; i < count; i++) {
    if(entries[i] != NULL && entries[i]->is_used() && entries[i]->get_id() == id) {
      return entries[i]->get_data();
    }
  }
  return NULL;
}

void Table::print_entries() {
  for (int i = 0; i < count; i++) {
    if(entries[i] != NULL && entries[i]->is_used()) {
      std::cout << "Entry with id " << entries[i]->get_id() << std::endl;
    }
  }
}

prod_table* ProducersTable::get_entry(mem_addr addr) {
  for (int i = 0; i < this->count; i++) {
    prod_table* en = (prod_table *)this->entries[i]->get_data();
    if (en->addr == addr) {
      return en;
    }
  }
  return NULL;
}

cons_table* ConsumersTable::get_entry(mem_addr addr) {
  std::cout << "In child class" << std::endl;
  for (int i = 0; i < this->count; i++) {
    TableEntry* te = (TableEntry *)this->entries[i];
    if (te != NULL) {
      cons_table* en = (cons_table *)this->entries[i]->get_data();
      if (en->addr == addr) {
        return en;
      }
    }
  }
  return NULL;
}

bool ConsumersTable::add_entry(cons_table* en) {
  //std::cout << "Add entry in child class" << std::endl;
  for (int i = 0; i < this->count; i++) {
    if (this->entries[i] != NULL) {
      cons_table* enn = (cons_table *)this->entries[i]->get_data();
      if (enn != NULL) {
        if (enn->addr == en->addr) {
          std::cout << "Address already exists" << std::endl;
          return true;
        }
      }
    }
  }
  return Table::add_entry((void *)en, 0);
}

bool ProducersTable::add_entry(prod_table* en) {
  std::cout << "Add entry in child class" << std::endl;
  for (int i = 0; i < this->count; i++) {
    if (this->entries[i] != NULL) {
      prod_table* enn = (prod_table *)this->entries[i]->get_data();
      if (enn != NULL) {
        if (enn->addr == en->addr) {
          std::cout << "Address already exists" << std::endl;
          return true;
        }
      }
    }
  }
  return Table::add_entry((void *)en, 0);
}

void ProducersTable::print_entries() {
  for (int i = 0; i < this->count; i++) {
    if (this->entries[i] != NULL) {
      prod_table* en = (prod_table *)(this->entries[i]->get_data());
      if ( en != NULL) {
        std::cout << en->addr << std::endl;
      }
    }
  }
}

void ConsumersTable::print_entries() {
  for (int i = 0; i < this->count; i++) {
    if (this->entries[i] != NULL) {
      cons_table* en = (cons_table *)(this->entries[i]->get_data());
      if ( en != NULL) {
        std::cout << en->addr << std::endl;
      }
    }
  }
}

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
    task_table->print_entries();
  }


}

void nexus1::add_to_task_table(task* t) {
  TaskTableEntry* tte = new TaskTableEntry;
  tte->id = t->id;
  tte->t = t;
  tte->status = NEW;
  tte->num_of_deps = calculate_deps(t);
  while(!task_table->add_entry((void *)tte, tte->id)) {
    //PRINTL("Task %d waiting for task table", tte->id);
    wait();
  }
}

int nexus1::calculate_deps(task* t) {
  // Here we fill data in producers and consumers tables
  int deps = 0;

  for (int i = 0; i < t->input_args; i++) {
    // Process each input arg
    cons_table* p = (cons_table *)consumers_table->get_entry(t->get_input_arg(i));
    if (p == NULL) {
      p = new cons_table;
      p->addr = t->get_input_arg(i);
      while(!consumers_table->add_entry(p)) {
        wait();
      }
    }
  }

  for (int i = 0; i < t->output_args; i++) {
    // Process each output arg
    prod_table* p = new prod_table;
    p->addr = t->get_output_arg(i);
    while (!producers_table->add_entry(p)) {
      wait();
    }
  }
  return 0;
}

void nexus1::schedule() {

}
