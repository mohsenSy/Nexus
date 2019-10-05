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
          //int segment_index = find_memory_segment_index(a);
          bus_memory_request.write(true);
          do {
            wait();
          } while(!bus_memory_accept.read());
          bus_memory_request.write(false);
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


/*void memory_segment::remote_receive() {
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
            PRINTL("Error, address %d is not here, min_addr: %d, max_addr: %d", a, min_addr, max_addr);
            //sc_stop();
          }
        }
        remote_memory_accept[i].write(false);
      }
    }
    wait();
  }
}*/

/*void memory_segment::bus_receive() {
  bus_addr_f.write(false);
  bus_data_v.write(false);
  bus_data_f.write(false);
  PRINTL("reset memory_segment bus_receive", "");
  while(true) {
    PRINTL("Check for memory request from bus", "");
    if (memory_request.read()) {
      PRINTL("Got memory request", "");
      memory_accept.write(true);
      do {
        wait();
      } while(!bus_addr_v.read());
      mem_addr a = bus_addr.read();
      bus_addr_f.write(true);
      do {
        wait();
      } while(bus_addr_v.read());
      bus_addr_f.write(false);
      if (a >= min_addr && a <= max_addr) {
        int d = 12;
        bus_data_v.write(true);
        bus_data.write(d);
        do {
          wait();
        } while(!bus_data_f.read());
      }
      else {
        PRINTL("Error, address %d is not here, min_addr: %d, max_addr: %d", a, min_addr, max_addr);
        sc_stop();
      }
      bus_data_v.write(false);
      memory_accept.write(false);
    }
    wait();
  }
}*/

void memory_bus::receive() {
  addr_f.write(false);
  data_v.write(false);
  data_f.write(false);
  remote_addr_v.write(false);
  //remote_data_v.write(false);
  //remote_data_f.write(false);
  PRINTL("Reset memory_bus receive", "");

  while(true) {
    for (int i = 0; i < NUMA_NODES; i++) {
      if (memory_request_r[i].read()) {
        memory_accept_s[i].write(true);
        do {
          wait();
        } while(!addr_v.read());
        mem_addr a = addr.read();
        addr_f.write(true);
        do {
          wait();
        } while(addr_v.read());
        int aa = *(int *)&a;
        /*int segment_index = aa / MEMORY_SEGMENT_SIZE;
        memory_request_s[segment_index].write(true);
        do {
          wait();
        } while(!memory_accept_r[segment_index].read());
        remote_addr_v.write(true);
        remote_addr.write(a);
        do {
          wait();
        } while(!remote_addr_f.read());
        remote_addr_v.write(false);
        do {
          wait();
        } while(!remote_data_v.read());
        int d = remote_data.read();*/
        int d = 90;
        for (int i = 0;i < MEMORY_BUS_DELAY-1; i++) {
          wait();
        }
        data.write(d);
        data_v.write(true);
        do {
          wait();
        } while(!data_f.read());
        data_v.write(false);
        memory_accept_s[i].write(false);
        //memory_request_s[segment_index].write(false);
      }
    }
    wait();
  }
}
