#include <memory_segment.h>
#include <parameters.h>
#include <utils.h>
#include <stats.h>

void memory_segment::core_receive() {
  // Initialization
  // Did not finish reading new address
  core_addr_f.write(false);
  core_data_v.write(false);
  core_data_f.write(false);
  while (true) {
    if (core_addr_v.read()) {
      if (core_rw.read()) {
        mem_addr a = core_addr.read();
        core_addr_f.write(true);
        do {
          wait();
        } while(core_addr_v.read());
        if (a > min_addr && a < max_addr) {
          int data = 89;
          core_data_v.write(true);
          core_data.write(data);
          do {
            wait();
          } while(!core_data_f.read());
          core_data_v.write(false);
        }
        else {
          PRINTL("Address not found here min_addr: %d, max_addr: %d, addr: %d",min_addr, max_addr, a);
          int segment_index = find_memory_segment_index(a);
          PRINTL("Address %d should be here %d", a, segment_index);
          remote_memory_request[segment_index].write(true);
          PRINTL("Memory segment %d requesting address from %d", index, segment_index);
          do {
            wait();
          } while(!remote_memory_accept[segment_index].read());
          remote_rw.write(true);
          remote_addr_v.write(true);
          remote_addr.write(a);
          do {
            wait();
          } while(!remote_addr_f.read());
          remote_addr_v.write(false);
          do {
            wait();
          } while(!remote_data_v.read());
          int d = remote_data.read();
          core_data_v.write(true);
          core_data.write(d);
          do {
            wait();
          } while(!core_data_f.read());
          core_data_v.write(false);
        }
      }
    }
    wait();
  }
}

int memory_segment::find_memory_segment_index(mem_addr addr) {
  int a = *(int *)&addr;
  return a / MEMORY_SEGMENT_SIZE;
}


void memory_segment::remote_receive() {
  // Initialization
  // Did not finish reading new address
  remote_addr_f.write(false);
  remote_data_f.write(false);
  remote_data_v.write(false);
  while (true) {
    for (int i = 0; i < NUMA_NODES; i++) {
      if (i == index) {
        continue;
      }
      PRINTL("Dumping memory requests in %d", index);
      for (int i = 0; i < NUMA_NODES; i++) {
        PRINTL("Request from %d is %d", i, remote_memory_request[i].read());
      }
      //PRINTL("Memroy segment %d check from segment %d", index, i);
      if (remote_memory_request[i].read()) {
        PRINTL("Memory segment %d received a memory request from %d", index, i);
        remote_memory_accept[i].write(true);
        do {
          PRINTL("Wait for address from %d", i);
          wait();
        } while(!remote_addr_v.read());
        if (remote_rw.read()) {
          mem_addr a = remote_addr.read();
          remote_addr_f.write(true);
          do {
            wait();
          } while(remote_addr_v.read());
          remote_addr_f.write(false);
          do {
            PRINTL("Wait to deactivate remote address valid in %d", index);
            wait();
          } while(remote_addr_v.read());
          if (a >= min_addr && a <= max_addr) {
            int d = 90;
            remote_data_v.write(true);
            remote_data.write(d);
            PRINTL("Sending data for address %d from segment %d", a, index);
            do {
              wait();
            } while(!remote_data_f.read());
            remote_data_v.write(false);
          }else {
            PRINTL("Fatal error, address %d is not here, min_addr: %d, max_addr: %d", a, min_addr, max_addr);
            sc_stop();
          }
        }
      }
    }
    wait();
  }
}
