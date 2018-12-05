#include <systemc.h>

#include <task.h>
#include <nexus.h>
#include <types.h>

class nexus1Helper {
public:
  sc_signal<bool> clock;
  sc_signal<task> t_in_sig;
  sc_signal<bool> t_in_v_sig;
  sc_signal<bool> t_in_f_sig;
  sc_signal<task> t_f_in_sig;
  sc_signal<bool> t_f_in_v_sig;
  sc_signal<bool> t_f_in_f_sig;
  sc_signal<task> t_out_sig;
  sc_signal<bool> t_out_v_sig;
  sc_signal<bool> t_out_f_sig;
  sc_signal<bool> rdy_sig;

  nexus1 *n1;
  nexus1Helper(sc_module_name name) {
    clock = 0;

    n1 = new nexus1(name);
    n1->clk(clock);

    n1->t_in(t_in_sig);
    n1->t_in_v(t_in_v_sig);
    n1->t_in_f(t_in_f_sig);

    n1->t_f_in(t_f_in_sig);
    n1->t_f_in_v(t_f_in_v_sig);
    n1->t_f_in_f(t_f_in_f_sig);

    n1->t_out(t_out_sig);
    n1->t_out_v(t_out_v_sig);
    n1->t_out_f(t_out_f_sig);

    n1->rdy(rdy_sig);

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
    // Make sure the nexus unit is ready to receive a new task
    while(rdy_sig == false) {
      cout << "Waiting for RDY" << std::endl;
      wait();
    }
    //std::cout << "Sending task " << t.id << std::endl;
    t_in_v_sig = true;
    t_in_sig = t;
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
    t_out_f_sig = false;
    t_f_in_v_sig = false;
    for(std::vector<task>::const_iterator iter = tasks.begin(); iter != tasks.end(); ++iter) {
      //std::cout<<iter->id<< std::endl;
      send_task(*iter);
    }
    //std::cout << "Sent all tasks" << std::endl;
    int i = 0;
    t_out_f_sig = true;
    while (i++ != 1000) {
      if (t_out_v_sig == true) {
        t_out_f_sig = false;
        wait();
        task t = t_out_sig;
        std::cout << "Got new task " << t.id << std::endl;
        wait();
        t_f_in_v_sig = true;
        t_f_in_sig = t;
        while (t_f_in_f_sig == false) {
          wait();
          std::cout << "Wait" << std::endl;
        }
        std::cout << "Done" << std::endl;
        t_f_in_v_sig = false;
        t_out_f_sig = true;
        wait();
      }
      /*else {
        std::cout << "No task" << std::endl;
      }*/
      wait();
    }
    /*while (rdy_sig != true) {
      std::cout << "Waiting for rdy_sig" << std::endl;
      wait();
    }*/
    sc_stop();
  }

};


int sc_main(int argc, char **argv) {
  nexus1Helper *n1H = new nexus1Helper("nexus1");
  std::string fileName = "m.csv";
  //cout << "Enter file name: ";
  //cin >> fileName;
  std::vector<task> tasks;

  read_tasks(fileName, &tasks);

  n1H->run(tasks);
  return 0;
}
