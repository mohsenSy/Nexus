#ifndef _TASK_H
#define _TASK_H

#include <types.h>

typedef struct task_descriptor {
  t_id id;
  int delay;
  int input_args, output_args;
  mem_addr *in_args;
  mem_addr *out_args;

  friend ostream& operator << (const ostream& os, const task_descriptor& t) {}

  friend std::istream &operator >>(std::istream & in, task_descriptor & td) {
    char c;
    in >> td.id >> c >> td.delay >> c >> td.input_args >> c >> td.output_args;;
    return in;
  }
}task;

inline bool operator ==(const task &t1, const task &t2) {
  return t1.id == t2.id;
}

inline bool operator !=(const task &t1, const task &t2) {
  return t1.id != t2.id;
}

extern void sc_trace(sc_trace_file *tf, const task& v, const std::string& NAME);

#endif
