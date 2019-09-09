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

#include <iostream>
#include <list>

using namespace std;

template <class T>
class Table {
  private:
    int size;
    typename list<T>::iterator get(const T &en) {
      for (auto it = entries.begin(); it != entries.end(); it++) {
        if (*it == en) {
          return it;
        }
      }
      return entries.end();
    }
  public:
    typename list<T>::iterator begin() {
      return entries.begin();
    }
    typename list<T>::iterator end() {
      return entries.end();
    }
    list<T> entries;
    T get_entry(const T &en) {
      auto it = get(en);
      if (it != entries.end()) {
        return *it;
      }
      return T();
    }
    T *find_entry(const T &en) {
      auto it = get(en);
      if (it != entries.end()) {
        return &(*it);
      }
      return nullptr;
    }
    bool add_entry(const T &en) {
      if (entries.size() < size) {
        entries.push_back(en);
        return true;
      }
      return false;
    }
    void delete_entry(const T &en) {
      entries.remove(en);
    }
    void set_entry(const T& old, const T& en) {
      T *t = find_entry(old);
      if (t) {
        *t = en;
      }
    }

    void dump(ostream& out = cout) {
      for (auto it = entries.begin(); it != entries.end(); it++) {
        out << *it << endl;
      }
    }
    Table(int n) {
      size = n;
    }
};
#endif
