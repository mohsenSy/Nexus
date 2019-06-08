#include <systemc.h>

#include <task.h>
#include <core.h>
#include <memory.h>
#include <types.h>

class coreHelper {
public:
  sc_signal<bool> clock;
  sc_signal<task> t_in_sig;
  sc_signal<bool> t_in_v_sig;
  sc_signal<bool> t_in_f_sig;
  sc_signal<task> t_out_sig;
  sc_signal<bool> t_out_v_sig;
  sc_signal<bool> t_out_f_sig;
  sc_signal<bool> rdy_sig;
  sc_signal<bool, SC_MANY_WRITERS> memory_rdy_sig;
  sc_signal<mem_addr, SC_MANY_WRITERS> memory_addr_sig;
  sc_signal<bool, SC_MANY_WRITERS> memory_addr_v_sig;
  sc_signal<bool, SC_MANY_WRITERS> memory_addr_f_sig;
  sc_signal<bool, SC_MANY_WRITERS> memory_addr_rdy_sig;

  memory *m;
  core *c;
  coreHelper(sc_module_name name) {
    clock = 0;
    m = new memory("memory_controller");

    m->clk(clock);
    m->rdy(memory_rdy_sig);
    m->addr(memory_addr_sig);
    m->addr_v(memory_addr_v_sig);
    m->addr_f(memory_addr_f_sig);
    m->addr_rdy(memory_addr_rdy_sig);

    c = new core(name);
    c->clk(clock);

    c->t_in(t_in_sig);
    c->t_in_v(t_in_v_sig);
    c->t_in_f(t_in_f_sig);

    c->t_out(t_out_sig);
    c->t_out_v(t_out_v_sig);
    c->t_out_f(t_out_f_sig);

    c->rdy(rdy_sig);
    c->memory_rdy(memory_rdy_sig);
    c->memory_addr(memory_addr_sig);
    c->memory_addr_v(memory_addr_v_sig);
    c->memory_addr_f(memory_addr_f_sig);
    c->memory_addr_rdy(memory_addr_rdy_sig);
  }

  void wait() {
    clock = (clock + 1) % 2;
    sc_start(1, SC_NS);
    if (clock == 1) {
      clock = (clock + 1) % 2;
      sc_start(1, SC_NS);
    }
  }

  void send_task(const task t) {
    // Make sure the core unit is ready to receive a new task
    while(rdy_sig == false) {
      wait();
    }
    t_in_v_sig = true;
    t_in_sig = t;
    wait();
    // Make sure the task is read by core unit
    while (t_in_f_sig != true) {
      wait();
    }
    t_in_v_sig = false;
    wait();
  }

  void run(const std::vector<task> tasks) {
    t_out_f_sig = true;
    for(std::vector<task>::const_iterator iter = tasks.begin(); iter != tasks.end(); ++iter) {
      //std::cout<<iter->id<< std::endl;
      send_task(*iter);
    }
    int i = 0;
    while (i++ != 1000) {
      wait();
    }
    std::cout << "Number of memory cycles is " << c->mem_cycles << std::endl;
    sc_stop();
  }

};


int sc_main(int argc, char **argv) {

  coreHelper *cH = new coreHelper("core1");
  std::string fileName = "m.csv";
  std::vector<task> tasks;
  read_tasks(fileName, &tasks);
  cH->run(tasks);
  return 0;
}
