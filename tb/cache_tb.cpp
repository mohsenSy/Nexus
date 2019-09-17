#include <systemc.h>

#include <parameters.h>
#include <cache.h>
#include <memory.h>
#include <types.h>

class cacheHelper {
public:
  sc_signal<bool> clock;
  sc_signal<bool> addr_v_sig;
  sc_signal<bool> addr_f_sig;
  sc_signal<mem_addr> addr_sig;
  sc_signal<bool> data_v_sig;
  sc_signal<bool> data_f_sig;
  sc_signal<sc_int<32> > data_sig;
  sc_signal<bool> rw_sig;

  l1cache *l1;
  cacheHelper(sc_module_name name) {
    clock = 0;
    l1 = new l1cache("l1cache");

    l1->clk(clock);
    l1->addr_v(addr_v_sig);
    l1->addr_f(addr_f_sig);
    l1->addr(addr_sig);
    l1->data_v(data_v_sig);
    l1->data_f(data_f_sig);
    l1->data(data_sig);
    l1->rw(rw_sig);

  }

  void wait() {
    clock = (clock + 1) % 2;
    sc_start(1, SC_NS);
    if (clock == 1) {
      clock = (clock + 1) % 2;
      sc_start(1, SC_NS);
    }
  }

  int read_addr(mem_addr a) {
    // Request a read process
    rw_sig.write(true);
    data_f_sig.write(false);
    addr_v_sig.write(true);
    addr_sig.write(a);
    std::cout << sc_time_stamp() << " Request memory address 1" << std::endl;
    do {
      wait();
    } while(!addr_f_sig.read());
    addr_v_sig.write(false);
    do {
      wait();
    } while(!data_v_sig.read());
    int d = data_sig.read();
    data_f_sig.write(true);
    do {
      wait();
    } while(data_v_sig.read());
    return d;
  }
  void write_addr(mem_addr a, int d) {
    // Request a read process
    rw_sig.write(false);
    addr_v_sig.write(true);
    addr_sig.write(a);
    std::cout << sc_time_stamp() << " Request memory address 1" << std::endl;
    do {
      wait();
    } while(!addr_f_sig.read());
    addr_v_sig.write(false);
    wait();
    data_v_sig.write(true);
    data_sig.write(d);
    do {
      wait();
    } while(!data_f_sig.read());
    data_v_sig.write(false);
    wait();
  }

};


int sc_main(int argc, char **argv) {

  cacheHelper *cH = new cacheHelper("l1cache");
  //std::string fileName = "nexus_tasks.csv";
  //std::vector<task> tasks;
  //read_tasks(fileName, &tasks);
  cH->write_addr((mem_addr)1, 200);
  int d = cH->read_addr((mem_addr)1);
  std::cout << "Got data " << d << " from l1cache" << std::endl;
  sc_stop();
  return 0;
}
