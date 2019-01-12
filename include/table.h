/*
  File name: table.h
  written by: Mouhsen Ibrahim <mohsen47@hotmail.co.uk>
  created at: 09:23 2019-01-04 UTC +3
  Description:
    This file was moved from nexus.h and contains template classes
    for tables and their entries.
*/
#ifndef __TABLE__H
#define __TABLE__H

#include <vector>
#include <iostream>

#include <types.h>
#include <task.h>
#include <parameters.h>

template <class T>
class TableEntry {
  private:
    bool used; // true when the entry is used
    int id;
    T *data;
  public:
    TableEntry(int id) {
      this->id = id;
      used = false;
      data = nullptr;
    }

    TableEntry(int id, T *data) {
      this->id = id;
      used = false;
      this->data = data;
    }
    // Returns true when the entry is used and false otherwise
    bool get_used(){
      return this->used;
    }
    int get_id() {
      return this->id;
    }
    T *get_data() {
      return this->data;
    }

    // Sets the used attribute
    void set_used(bool used) {
      this->used = used;
    }
    void set_id(int id) {
      this->id = id;
    }
    void set_data(T* data) {
      this->data = data;
    }
};


template <class T>
class Table {
  protected:
    TableEntry<T>** entries;
    int count;
    TableEntry<T> *get_entry(int id) {
      for (int i = 0; i < count; i++) {
        if (entries[i] != nullptr && entries[i]->get_data() != nullptr && entries[i]->get_used() && entries[i]->get_id() == id) {
          return entries[i];
        }
      }
      return nullptr;
    }
  public:
    Table(int count) {
      entries = new TableEntry<T>*[count];
      this->count = count;
    }

    int get_count() {
      return this->count;
    }

    T *operator[](int index) {
      if (index < 0) {
        return nullptr;
      }
      if (entries[index] != nullptr) {
        return entries[index]->get_data();
      }
      return nullptr;
    }
    bool add_entry(T *en, int id) {
      // Adds en to the array of entries
      // Find a valid entry
      for (int i = 0; i < count; i++) {
        if (entries[i] == NULL) {
          // Add a new Entry
          TableEntry<T> *te = new TableEntry<T>(id, en);
          te->set_used(true);
          te->set_data(en);
          entries[i] = te;
          return true;
        }
        if (!entries[i]->get_used()) {
          // The entry exists and is not used
          entries[i]->set_data(en);
          entries[i]->set_used(true);
          entries[i]->set_id(id);
          return true;
        }
      }
      return false;
    }
    T *get_data(int id) {
      TableEntry<T> *t = this->get_entry(id);
      return t == nullptr ? nullptr : t->get_data();
    }
    bool delete_entry(int id) {
      TableEntry<T> *t = this->get_entry(id);
      if (t == nullptr) {
        return false;
      }
      t->set_data(nullptr);
      t->set_used(false);
      return true;
    }
    void print_entries() {
      for (int i = 0; i < count ; i++) {
        if (entries[i] != nullptr && entries[i]->get_data() != nullptr && entries[i]->get_used() == true) {
          std::cout << "id: " << entries[i]->get_id() << std::endl;
          entries[i]->get_data()->print();
          std::cout << "=========" << std::endl;
        }
      }
    }
};
#endif
