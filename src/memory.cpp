#include <memory.h>
#include <parameters.h>
#include <utils.h>

void memory::do_fetch() {
  // Initialization
  // Ready to receive new address
  rdy.write(true);
  // Did not finish reading new address
  addr_f.write(false);

  while (true) {
    // Check if address input is valid?
    if (addr_v.read()) {
        mem_addr a = addr.read();
        addr_f.write(true);
        rdy.write(false);
        wait();
        for (int i = 0; i < MEM_FETCH_TIME; i++) {
          wait();
        }
        addr_f.write(false);
        rdy.write(true);
        wait();
    }
    wait();
  }
}
