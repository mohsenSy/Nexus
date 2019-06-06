#include <systemc.h>

#include <task.h>
#include <execute.h>
#include <utils.h>

class executeHelper {
public:
  sc_signal<bool> clock;
  sc_signal<task> t_in_sig;
  sc_signal<bool> t_in_v_sig;
  sc_signal<bool> t_in_f_sig;
  sc_signal<task> t_out_sig;
  sc_signal<bool> t_out_v_sig;
  sc_signal<bool> t_out_f_sig;
  sc_signal<bool> rdy_sig;

  execute *ex;
  executeHelper(sc_module_name name) {
    ex = new execute(name);
    clock = 0;
    ex->clk(clock);

    ex->t_in(t_in_sig);
    ex->t_in_v(t_in_v_sig);
    ex->t_in_f(t_in_f_sig);

    ex->t_out(t_out_sig);
    ex->t_out_v(t_out_v_sig);
    ex->t_out_f(t_out_f_sig);

    ex->rdy(rdy_sig);
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
    // Make sure the execution unit is ready to receive a new task
    while(rdy_sig != true) {
      wait();
    }
    t_in_v_sig = true;
    t_in_sig = t;
    wait();
    // Make sure the task is read by execution unit
    do {
      wait();
    } while(t_in_f_sig == false);
    t_in_v_sig = false;
    // std::cout << "Sent task " << t.id << std::endl;
    wait();
  }

  void run(const std::vector<task> tasks) {
    for(std::vector<task>::const_iterator iter = tasks.begin(); iter != tasks.end(); ++iter) {
      t_out_f_sig = false;
      wait();
      //std::cout<<iter->id<< std::endl;
      send_task(*iter);
      while (t_out_v_sig == false) {
        wait();
      }
      t_out_f_sig = true;
      task t = t_out_sig;
      wait();
    }
    while (rdy_sig != true) {
      wait();
    }
    sc_stop();
  }

};


int sc_main(int argc, char **argv) {


  executeHelper *exH = new executeHelper("execute unit 0");
  std::string fileName = "m.csv";
  std::vector<task> tasks;

  read_tasks(fileName, &tasks);

  exH->run(tasks);


  return 0;
}
