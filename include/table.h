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

#include <systemc.h>


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
    //sc_mutex* m;
  public:
    TableEntry(int id) {
      this->id = id;
      used = false;
      data = nullptr;
      //m = new sc_mutex();
    }

    TableEntry(int id, T *data) {
      this->id = id;
      used = data == nullptr ? false : true;
      this->data = data;
      //m = new sc_mutex();
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
      //m->lock();
      this->used = used;
      //m->unlock();
    }
    void set_id(int id) {
      //m->lock();
      this->id = id;
      //m->unlock();
    }
    void set_data(T* data) {
      //m->lock();
      this->data = data;
      //m->unlock();
    }
};


template <class T>
class Table {
  //private:
    //sc_mutex* m;
  protected:
    TableEntry<T>** entries;
    int count;
    TableEntry<T> *get_entry(int id) {
      //m->lock();
      for (int i = 0; i < count; i++) {
        if (entries[i] != nullptr && entries[i]->get_data() != nullptr && entries[i]->get_used() && entries[i]->get_id() == id) {
          //m->unlock();
          return entries[i];
        }
      }
      //m->unlock();
      return nullptr;
    }
  public:
    Table(int count) {
      entries = new TableEntry<T>*[count];
      this->count = count;
      //m = new sc_mutex();
    }

    int get_count() {
      return this->count;
    }

    bool has_empty_entries() {
      //m->lock();
      for (int i = 0; i < count; i++) {
        if (entries[i] == nullptr) {
          return true;
        }
        if (!entries[i]->get_used()) {
          return true;
        }
      }
      //m->unlock();
      return false;
    }

    T *operator[](int index) {
      //m->lock();
      if (index < 0) {
        //m->unlock();
        return nullptr;
      }
      if (entries[index] != nullptr) {
        //m->unlock();
        return entries[index]->get_data();
      }
      //m->unlock();
      return nullptr;
    }
    bool add_entry(T *en, int id) {
      // Adds en to the array of entries
      // Find a valid entry
      //m->lock();
      for (int i = 0; i < count; i++) {
        if (entries[i] == nullptr) {
          // Add a new Entry
          TableEntry<T> *te = new TableEntry<T>(id, en);
          //te->set_used(true);
          //te->set_data(en);
          entries[i] = te;
          //m->unlock();
          return true;
        }
        if (!entries[i]->get_used()) {
          // The entry exists and is not used
          entries[i]->set_data(en);
          entries[i]->set_used(true);
          entries[i]->set_id(id);
          //m->unlock();
          return true;
        }
      }
      //m->unlock();
      return false;
    }
    T *get_data(int id) {
      TableEntry<T> *t = this->get_entry(id);
      return t == nullptr ? nullptr : t->get_data();
    }
    bool delete_entry(int id) {
      //m->lock();
      TableEntry<T> *t = this->get_entry(id);
      if (t == nullptr) {
        //m->unlock();
        return false;
      }
      t->set_data(nullptr);
      t->set_used(false);
      //m->unlock();
      return true;
    }
    void print_entries() {
      for (int i = 0; i < count ; i++) {
        if (entries[i] != nullptr && entries[i]->get_data() != nullptr && entries[i]->get_used() == true) {
          std::cout << "index: " << i << std::endl;
          std::cout << "id: " << entries[i]->get_id() << std::endl;
          entries[i]->get_data()->print();
          std::cout << "=========" << std::endl;
        }
      }
    }
};
#endif
