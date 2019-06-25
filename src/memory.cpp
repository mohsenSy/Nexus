#include <memory.h>
#include <parameters.h>
#include <utils.h>
#include <stats.h>

void memory::do_fetch() {
  // Initialization
  // Ready to receive new address
  rdy.write(true);
  // Did not finish reading new address
  addr_f.write(false);

  // no data is ready on output
  data_rdy.write(false);
  for(int i = 0; i < CORE_NUM; i++) {
    core_memory_accept[i].write(false);
  }

  while (true) {
    // Check if address input is valid?
    for (int i = 0; i < CORE_NUM; i++) {
      if (core_memory_request[i].read()) {
        core_memory_accept[i].write(true);
        while(!addr_v.read()) {
          wait();
        }
        mem_addr a = addr.read();
        //PRINTL("Reading input %d", a);
        addr_f.write(true);
        rdy.write(false);
        wait();
        for (int i = 0; i < MEM_FETCH_TIME; i++) {
          wait();
        }
        addr_f.write(false);
        rdy.write(true);
        data_rdy.write(true);
        wait();
      }
      data_rdy.write(false);
      core_memory_accept[i].write(false);
      wait();
    }
    data_rdy.write(false);
    wait();
  }
}
