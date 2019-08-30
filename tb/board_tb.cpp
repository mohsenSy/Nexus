#include <systemc.h>
#include<string>
#include<fstream>
#include<vector>

#include <task.h>
#include <board.h>
#include <stats.h>

class boardHelper {
public:
  sc_signal<bool> clock;
  sc_signal<task> t_in_sig;
  sc_signal<bool> t_in_v_sig;
  sc_signal<bool> t_in_f_sig;
  sc_signal<bool> rdy_sig;
  board *b;
  boardHelper(sc_module_name name) {
    b = new board(name);
    clock = 0;
    b->clk(clock);

    b->t_in(t_in_sig);
    b->t_in_v(t_in_v_sig);
    b->t_in_f(t_in_f_sig);

    b->rdy(rdy_sig);
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
    // Make sure the board unit is ready to receive a new task
    while(rdy_sig != true) {
      cout << "Waiting for RDY" << std::endl;
      wait();
    }
    t_in_v_sig = true;
    t_in_sig = t;
    // std::cout << "Sending task " << t.id << std::endl;
    wait();
    // Make sure the task is read by core board
    while (t_in_f_sig != true) {
      wait();
    }
    t_in_v_sig = false;
    // std::cout << "Sent task with id " << t.id << std::endl;
  }

  void run(const std::vector<task> tasks) {
    for(std::vector<task>::const_iterator iter = tasks.begin(); iter != tasks.end(); ++iter) {
      // std::cout<<iter->id<< std::endl;
      send_task(*iter);
    }
    // std::cout << "Sent all tasks" << std::endl;
    int i = 0;
    while (i++ != 100000) {
      wait();
    }
    /*while (rdy_sig != true) {
      std::cout << "Waiting for rdy_sig" << std::endl;
      wait();
    }*/
    Stats::print_core_finished_tasks_num();
    Stats::print_core_wait_cycles();
    Stats::print_execute_cycles();
    Stats::print_memory_cycles();
    Stats::print_board_buffer_wait_cycles();
    sc_stop();
  }

};

int sc_main(int argc, char **argv) {

  /*sc_signal<bool> clock;
  sc_signal<task> t_in_sig;
  sc_signal<bool> t_in_v_sig;
  sc_signal<bool> t_in_f_sig;
  sc_signal<bool> rdy_sig;*/
  std::string fileName = "tasks.csv";
  //cout << "Enter file name: ";
  //cin >> fileName;
  std::vector<task> tasks;

  read_tasks(fileName, &tasks);

  //board b1("board1");
  boardHelper *bH = new boardHelper("board1");

  bH->run(tasks);

  /*b1.clk(clock);
  b1.t_in(t_in_sig);
  b1.t_in_v(t_in_v_sig);
  b1.t_in_f(t_in_f_sig);
  b1.rdy(rdy_sig);

  //t_in_sig = t_in;
  t_in_v_sig = true;
  clock = 0;
  for(std::vector<task>::const_iterator iter = tasks.begin(); iter != tasks.end(); ++iter) {
    //std::cout<<iter->id<<" "<<iter->func_pointer<<" "<<iter->input_args<<" " <<iter->output_args<<std::endl;
    clock = (clock + 1) % 2;
    t_in_sig = *iter;
    sc_start(1, SC_NS);
  }

  for (int i = 0; i < 100; i++) {
    clock = (clock + 1) % 2;
    sc_start(1, SC_NS);
  }
  cout << "Finished" << endl;

  sc_stop();
  cout << "Stopped" << endl;*/

  return 0;
}
