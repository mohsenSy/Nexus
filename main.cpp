#include <task.h>
#include <core.h>
#include <systemc.h>

int sc_main(int args, char **argv) {

  sc_signal<task> t_sig;
  int delay = 10;

  task t;
  t.func_pointer = (void*)&delay;

  core* c = new core();

  c.t_in(t_sig);

  sc_start(20,SC_NS);

  t_sig = t;

  sc_stop();

  return 0;
}
