#include <systemc.h>
#include <task.h>
#include <cstdarg>


void task_descriptor::set_input_arg(int index, mem_addr addr) {
  if (in_args == NULL) {
    in_args = new mem_addr[this->input_args];
  }
  if (index >= 0 && index < this->input_args) {
    in_args[index] = addr;
  }
}


void task_descriptor::set_output_arg(int index, mem_addr addr) {
  if (out_args == NULL) {
    out_args = new mem_addr[this->output_args];
  }
  if (index >= 0 && index < this->output_args) {
    out_args[index] = addr;
  }
}

mem_addr task_descriptor::get_input_arg(int i) {
  // Make sure the required index exists
  if (i >= this->input_args | i < 0) {
    return NULL;
  }
  // Make sure the input args were set previously
  if (this->in_args == NULL) {
    return NULL;
  }
  // Return the required input argument
  return this->in_args[i];
}

mem_addr task_descriptor::get_output_arg(int i) {
  // Make sure the required index exists
  if (i >= this->output_args | i < 0) {
    return NULL;
  }
  // Make sure the output args were set previously
  if (this->out_args == NULL) {
    return NULL;
  }
  // Return the required output argument
  return this->out_args[i];
}

void sc_trace(sc_trace_file *tf, const task& v, const std::string& NAME){}
