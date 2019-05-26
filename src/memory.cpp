#include <memory.h>
#include <parameters.h>
#include <utils.h>

void memory::do_fetch() {
  // Initialization
  // Ready to receive new address
  rdy.write(true);
  // Did not finish reading new address
  addr_f.write(false);

  // no data is ready on output
  addr_rdy.write(false);

  while (true) {
    // Check if address input is valid?
    if (addr_v.read()) {
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
        addr_rdy.write(true);
        wait();
        //PRINTL("Finished reading input %d", a);
    }
    addr_rdy.write(false);
    wait();
  }
}
