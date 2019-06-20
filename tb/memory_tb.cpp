#include <systemc.h>

#include <task.h>
#include <core.h>
#include <memory.h>
#include <types.h>
#include <parameters.h>

class memoryHelper {
public:
  sc_signal<bool> clock;
  sc_signal<mem_addr> addr_sig;
  sc_signal<bool> addr_v_sig;
  sc_signal<bool> addr_f_sig;
  sc_signal<bool> data_rdy_sig;
  sc_signal<bool> rdy_sig;
  sc_vector<sc_signal<bool> > core_memory_request_sig;
  sc_vector<sc_signal<bool> > core_memory_accept_sig;
  //
  /*sc_signal<bool, SC_MANY_WRITERS> memory_rdy_sig;
  sc_signal<mem_addr, SC_MANY_WRITERS> memory_addr_sig;
  sc_signal<bool, SC_MANY_WRITERS> memory_addr_v_sig;
  sc_signal<bool, SC_MANY_WRITERS> memory_addr_f_sig;
  sc_signal<bool, SC_MANY_WRITERS> memory_addr_rdy_sig;*/

  memory *m;
  memoryHelper(sc_module_name name) {
    clock = 0;
    m = new memory(name);

    m->clk(clock);
    m->rdy(rdy_sig);
    m->addr(addr_sig);
    m->addr_v(addr_v_sig);
    m->addr_f(addr_f_sig);
    m->data_rdy(data_rdy_sig);
    core_memory_request_sig.init(CORE_NUM);
    core_memory_accept_sig.init(CORE_NUM);
    for (int i = 0; i < CORE_NUM; i++) {
      m->core_memory_request[i](core_memory_request_sig[i]);
      m->core_memory_accept[i](core_memory_accept_sig[i]);
    }

  }

  void wait() {
    clock = (clock + 1) % 2;
    sc_start(1, SC_NS);
    if (clock == 1) {
      clock = (clock + 1) % 2;
      sc_start(1, SC_NS);
    }
  }

  void send_addr(const mem_addr addr) {
    std::cout << sc_time_stamp() << std::endl;
    addr_v_sig = true;
    addr_sig = addr;
    core_memory_request_sig[0] = true;
    wait();
    while(core_memory_accept_sig[0] != true) {
      wait();
    }
    // Make sure the address is read by memory unit
    while (addr_f_sig != true) {
      wait();
    }
    addr_v_sig = false;
    wait();
    // wait until address data is read
    while(data_rdy_sig == false) {
      wait();
    }
    std::cout << sc_time_stamp() << std::endl;
  }

  void run() {
    mem_addr addr = (mem_addr)90;
    send_addr(addr);
    addr = (mem_addr)99;
    send_addr(addr);
    sc_stop();
  }

};


int sc_main(int argc, char **argv) {

  memoryHelper *mH = new memoryHelper("memory1");

  mH->run();

  return 0;
}
