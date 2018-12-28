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
      entries[i] = NULL;
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

TaskTableEntry* Table::get_ready_task() {
  for (int i = 0; i < count; i++) {
    if(entries[i] != NULL && entries[i]->is_used()) {
      TaskTableEntry* t = (TaskTableEntry *)entries[i]->get_data();
      if (t->status == NEW && t->num_of_deps == 0) {
        return t;
      }
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
    TableEntry* te = (TableEntry *)this->entries[i];
    if (te != NULL) {
      prod_table* en = (prod_table *)this->entries[i]->get_data();
      if (en->addr == addr) {
        return en;
      }
    }
  }
  return NULL;
}

cons_table* ConsumersTable::get_entry(mem_addr addr) {
  //std::cout << "In child class" << std::endl;
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
          //std::cout << "Address already exists" << std::endl;
          return true;
        }
      }
    }
  }
  return Table::add_entry((void *)en, 0);
}

bool ConsumersTable::delete_entry(cons_table* en) {
  for (int i = 0; i < this->count; i++) {
    if (this->entries[i] != NULL) {
      cons_table* enn = (cons_table *)this->entries[i]->get_data();
      if (enn != NULL) {
        if (enn->addr == en->addr) {
          //std::cout << "Address already exists" << std::endl;
          return true;
        }
      }
    }
  }
  return Table::add_entry((void *)en, 0);
}

bool ConsumersTable::delete_addr(mem_addr m) {
  for (int i = 0; i < this->count; i++) {

  }
  return true;
}

bool ProducersTable::add_entry(prod_table* en) {
  //std::cout << "Add entry in child class" << std::endl;
  for (int i = 0; i < this->count; i++) {
    if (this->entries[i] != NULL) {
      prod_table* enn = (prod_table *)this->entries[i]->get_data();
      if (enn != NULL) {
        if (enn->addr == en->addr) {
          //std::cout << "Address already exists" << std::endl;
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

void ConsumersTable::increment_deps(mem_addr addr) {
  cons_table* e = this->get_entry(addr);
  if (e) {
    e->num_of_deps++;
  }
}

void ConsumersTable::add_to_kick_off_list(mem_addr addr, task t) {
  cons_table* e = this->get_entry(addr);
  if (e) {
    if (e->index < NEXUS1_KICK_OFF_LIST_SIZE) {
      e->kick_of_list[e->index++] = t;
    }
  }
}

void ProducersTable::add_to_kick_off_list(mem_addr addr, task t) {
  prod_table* e = this->get_entry(addr);
  if (e) {
    if (e->index < NEXUS1_KICK_OFF_LIST_SIZE) {
      e->kick_of_list[e->index++] = t;
    }
  }
}
