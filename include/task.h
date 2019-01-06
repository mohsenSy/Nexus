#ifndef _TASK_H
#define _TASK_H
//#include <systemc>
#include <types.h>
#include <iostream>

typedef struct task_descriptor {
public:
  t_id id;
  int delay;
  int input_args, output_args;
  mem_addr* in_args;
  mem_addr* out_args;
  // This function must be removed
  void set_input_args(int n_args, ...);
  // This function must be removed
  void set_output_args(int n_args, ...);
  void set_input_arg(int, mem_addr);
  void set_output_arg(int, mem_addr);
  mem_addr get_input_arg(int index);
  mem_addr get_output_arg(int index);

  friend std::ostream& operator << (const std::ostream& os, const task_descriptor& t) {}

  friend std::istream& operator >>(std::istream & in, task_descriptor & td) {
    char c;
    in >> td.id >> c >> td.delay >> c >> td.input_args >> c >> td.output_args;
    return in;
  }
}task;

inline bool operator ==(const task &t1, const task &t2) {
  return t1.id == t2.id;
}

inline bool operator !=(const task &t1, const task &t2) {
  return t1.id != t2.id;
}

//extern void sc_trace(sc_trace_file *tf, const task& v, const std::string& NAME);

#endif
