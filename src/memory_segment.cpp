#include <memory_segment.h>
#include <parameters.h>
#include <utils.h>
#include <stats.h>

void memory_segment::receive() {
  // Initialization
  // Did not finish reading new address
  addr_f.write(false);
  while (true) {
    // Check if address input is valid?
    /*for (int i = 0; i < CORE_NUM; i++) {
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
    data_rdy.write(false);*/
    wait();
  }
}


void memory_segment::remote_receive() {
  // Initialization
  // Did not finish reading new address
  remote_addr_f.write(false);
  while (true) {
    // Check if address input is valid?
    /*for (int i = 0; i < CORE_NUM; i++) {
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
    data_rdy.write(false);*/
    wait();
  }
}
