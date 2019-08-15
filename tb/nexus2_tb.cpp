#include <systemc.h>

#include <task.h>
#include <nexus2.h>
#include <types.h>
#include <utils.h>

class nexus2Helper {
public:
  sc_signal<bool> clock;
  sc_signal<task> t_in_sig;
  sc_signal<bool> t_in_v_sig;
  sc_signal<bool> t_in_f_sig;
  /*sc_signal<task> t_f_in_sig;
  sc_signal<bool> t_f_in_v_sig;
  sc_signal<bool> t_f_in_f_sig;
  sc_signal<task> t_out_sig;
  sc_signal<bool> t_ready_sig;
  sc_signal<bool> t_out_v_sig;
  sc_signal<bool> t_out_f_sig;*/
  sc_signal<bool> rdy_sig;
  /*sc_signal<task> t_ready_out_sig;
  sc_signal<bool> t_ready_out_f_sig;
  sc_signal<bool> t_ready_out_v_sig;
  #ifdef DEBUG
  sc_signal<int> debug_sig;
  #endif*/

  nexus2::nexus *n2;
  nexus2Helper(sc_module_name name) {
    clock = 0;

    n2 = new nexus2::nexus(name);
    n2->clk(clock);

    n2->t_in(t_in_sig);
    n2->t_in_v(t_in_v_sig);
    n2->t_in_f(t_in_f_sig);
/*
    n1->t_f_in(t_f_in_sig);
    n1->t_f_in_v(t_f_in_v_sig);
    n1->t_f_in_f(t_f_in_f_sig);

    n1->t_out(t_out_sig);
    n1->t_ready(t_ready_sig);
    n1->t_out_v(t_out_v_sig);
    n1->t_out_f(t_out_f_sig);
*/
    n2->rdy(rdy_sig);
/*
    n1->t_ready_out(t_ready_out_sig);
    n1->t_ready_out_f(t_ready_out_f_sig);
    n1->t_ready_out_v(t_ready_out_v_sig);


    #ifdef DEBUG
    n1->debug(debug_sig);
    #endif
*/
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

  /*void finish_tasks(const std::vector<task> tasks) {
    for(std::vector<task>::const_iterator iter = tasks.begin(); iter != tasks.end(); ++iter) {
      //std::cout<<iter->id<< std::endl;
      t_f_in_v_sig = true;
      t_f_in_sig = *iter;
      do {
        wait();
      } while(t_f_in_f_sig != true);
      t_f_in_v_sig = true;
      wait();
    }
  }*/
  void run(const std::vector<task> tasks) {
    /*t_out_f_sig = false;
    t_f_in_v_sig = false;
    t_ready_out_f_sig = false;*/
    for(std::vector<task>::const_iterator iter = tasks.begin(); iter != tasks.end(); ++iter) {
      //std::cout<<iter->id<< std::endl;
      send_task(*iter);
    }
    /*
    //finish_tasks(tasks);
    while(!t_ready_out_v_sig.read()) {
      wait();
      std::cout << "Wait for ready task " << std::endl;
    }
    task t = t_ready_out_sig.read();
    std::cout << "task " << t.id << " is ready " << std::endl;
    int counter = 0;
    while(counter++ < 1000) {
      wait();
    }
    #ifdef DEBUG
    debug_sig.write(4);
    wait();
    debug_sig.write(5);
    wait();
    debug_sig.write(6);
    wait();
    debug_sig.write(0);
    #endif
    //std::cout << "Sent all tasks" << std::endl;
    /*int i = 0;
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
      }*/
      /*else {
        std::cout << "No task" << std::endl;
      }*/
      wait();
    //}
    /*while (rdy_sig != true) {
      std::cout << "Waiting for rdy_sig" << std::endl;
      wait();
    }*/
    sc_stop();
  }

};


int sc_main(int argc, char **argv) {
  nexus2Helper *n2H = new nexus2Helper("nexus2");
  std::string fileName = "nexus_tasks.csv";
  //cout << "Enter file name: ";
  //cin >> fileName;
  std::vector<task> tasks;

  read_tasks(fileName, &tasks);

  n2H->run(tasks);
  return 0;
}
