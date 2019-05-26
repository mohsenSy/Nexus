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
      //cout << "Waiting for RDY" << std::endl;
      wait();
    }
    t_in_v_sig = true;
    t_in_sig = t;
    //std::cout << "Sending task " << t.id << std::endl;
    wait();
    // Make sure the task is read by core unit
    while (t_in_f_sig != true) {
      wait();
    }
    //t_in_v_sig = false;
    wait();
    //std::cout << "Sent task with id " << t.id << std::endl;
  }

  void run(const std::vector<task> tasks) {
    t_out_f_sig = true;
    for(std::vector<task>::const_iterator iter = tasks.begin(); iter != tasks.end(); ++iter) {
      //std::cout<<iter->id<< std::endl;
      send_task(*iter);
    }
    //std::cout << "Sent all tasks" << std::endl;
    int i = 0;
    while (i++ != 1000) {
      wait();
    }
    /*while (rdy_sig != true) {
      std::cout << "Waiting for rdy_sig" << std::endl;
      wait();
    }*/
    std::cout << "Number of memory cycles is " << c->mem_cycles << std::endl;
    sc_stop();
  }

};


int sc_main(int argc, char **argv) {

  /*sc_signal<bool> clock;
  sc_signal<task> t_in_sig;
  sc_signal<bool> t_in_v_sig;
  sc_signal<bool> t_in_f_sig;
  sc_signal<task> t_out_sig;
  //sc_signal<bool, SC_MANY_WRITERS> t_out_v_sig;
  sc_signal<bool> t_out_v_sig;
  sc_signal<bool> t_out_f_sig;
  //sc_signal<bool, SC_MANY_WRITERS> rdy_sig;
  sc_signal<bool> rdy_sig;
  //sc_signal<bool, SC_MANY_WRITERS> finished_sig;

  task t_in;
  task t2_in;
  task t3_in;
  task t_out;
  t_in.id = 1;
  t_in.delay = 3;
  t2_in.id = 2;
  t2_in.delay = 2;
  t3_in.id = 3;
  t3_in.delay = 4;

  core c("core");

  c.t_in(t_in_sig);
  c.t_in_v(t_in_v_sig);
  c.t_in_f(t_in_f_sig);
  c.t_out(t_out_sig);
  c.t_out_v(t_out_v_sig);
  c.t_out_f(t_out_f_sig);
  c.rdy(rdy_sig);
  c.clk(clock);
  //c.finished(finished_sig);*/

  coreHelper *cH = new coreHelper("core1");
  std::string fileName = "m.csv";
  //cout << "Enter file name: ";
  //cin >> fileName;
  std::vector<task> tasks;

  read_tasks(fileName, &tasks);

  cH->run(tasks);


  /*clock = 0;
  sc_start(1, SC_NS);
  int i = 0;
  while(rdy_sig != true) {
    cout << "Waiting1 for core to be ready" << endl;
    clock = (clock + 1) % 2;
    sc_start(1, SC_NS);
    if(i++ == 10) {
      return 1;
    }
  }
  t_in_sig = t_in;
  t_in_v_sig = true;
  clock = (clock + 1) % 2;
  i = 0;
  while(rdy_sig != true) {
    cout << "Waiting2 for core to be ready" << endl;
    clock = (clock + 1) % 2;
    sc_start(1, SC_NS);
    if(i++ == 10) {
      return 1;
    }
  }
  t_in_sig = t3_in;
  sc_start(1, SC_NS);
  clock = (clock + 1) % 2;
  i = 0;
  while(rdy_sig != true) {
    cout << "Waiting3 for core to be ready" << endl;
    clock = (clock + 1) % 2;
    sc_start(1, SC_NS);
    if(i++ == 10) {
      return 1;
    }
  }
  t_in_sig = t2_in;
  sc_start(1, SC_NS);
  clock = (clock + 1) % 2;
  sc_start(1, SC_NS);
  clock = (clock + 1) % 2;
  sc_start(1, SC_NS);
  cout << "RDY is " << rdy_sig << endl;
  clock = (clock + 1) % 2;
  sc_start(1, SC_NS);
  clock = (clock + 1) % 2;
  sc_start(1, SC_NS);
  clock = (clock + 1) % 2;
  sc_start(1, SC_NS);
  clock = (clock + 1) % 2;
  sc_start(1, SC_NS);
  cout << "RDY is " << rdy_sig << endl;
  clock = (clock + 1) % 2;
  sc_start(1, SC_NS);
  clock = (clock + 1) % 2;
  sc_start(1, SC_NS);
  clock = (clock + 1) % 2;
  sc_start(1, SC_NS);
  cout << "RDY is " << rdy_sig << endl;
  cout << "Finished" << endl;

  sc_stop();*/

  return 0;
}
