#include <systemc.h>

#include <task.h>
#include <core.h>
#include <memory.h>
#include <types.h>

class memoryHelper {
public:
  sc_signal<bool> clock;
  sc_signal<mem_addr> addr_sig;
  sc_signal<bool> addr_v_sig;
  sc_signal<bool> addr_f_sig;
  sc_signal<bool> addr_rdy_sig;
  sc_signal<bool> rdy_sig;
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
    m->addr_rdy(addr_rdy_sig);
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
    // Make sure the memory unit is ready to receive a new address
    while(rdy_sig == false) {
      wait();
    }
    addr_v_sig = true;
    addr_sig = addr;
    wait();
    // Make sure the address is read by memory unit
    while (addr_f_sig != true) {
      wait();
    }
    addr_v_sig = false;
    wait();
    // wait until address data is read
    while(addr_rdy_sig == false) {
      wait();
    }
    std::cout << sc_time_stamp() << std::endl;
  }

  void run() {
    mem_addr addr = (mem_addr)90;
    send_addr(addr);
    sc_stop();
  }

};


int sc_main(int argc, char **argv) {

  memoryHelper *mH = new memoryHelper("memory1");

  mH->run();

  return 0;
}
