#include <cache.h>
#include <utils.h>

void l1cache::receive() {
  while(true) {
    bool read = rw.read();
    if (read) {
      read_data();
    }
    else {
      write_data();
    }
    wait();
  }
}

int l1cache::read_data_l2cache(mem_addr a) {
  memory_request.write(true);
  do {
    //PRINTL("memory accept", "");
    wait();
  } while(!memory_accept.read());
  l2_rw.write(true);
  l2_data_f.write(false);
  l2_addr_v.write(true);
  l2_addr.write(a);
  PRINTL("Wait l2_addr_f", "");
  do {
    //PRINTL("Wait l2_addr_f", "");
    wait();
  } while(!l2_addr_f.read());
  l2_addr_v.write(false);
  memory_request.write(false);
  do {
    wait();
  } while(!l2_data_v.read());
  int d = l2_data.read();
  l2_data_f.write(true);
  do {
    wait();
  } while(l2_data_v.read());
  return d;
}

void l1cache::read_data() {
  if (addr_v.read()) {
    mem_addr a = addr.read();
    addr_f.write(true);
    PRINTL("Reading data for address %d from l1cache", a);
    do {
      wait();
    } while(addr_v.read());
    addr_f.write(false);
    wait();
    int d = 0;
    AddressTableEntry ate = at.get_entry(AddressTableEntry(a));
    if (ate.get_addr() == 0) {
      // no address found
      PRINTL("Address %d not found", a);
      d = read_data_l2cache(a);
    }
    else {
      d = ate.get_data();
      PRINTL("Address %d found with data %d", a, d);
    }
    at.add_entry(AddressTableEntry(a, d));
    for (int i = 0; i < L1CACHEDELAY-1; i++) {
      wait();
    }
    data_v.write(true);
    data.write(d);
    wait();
    do {
      wait();
    } while(!data_f.read());
    data_v.write(false);
    wait();
  }
  else {
    wait();
    return;
  }
}

void l1cache::write_data() {
  if (addr_v.read()) {
    mem_addr a = addr.read();
    addr_f.write(true);
    PRINTL("Writing data for address %d", a);
    do {
      wait();
    } while(addr_v.read());
    addr_f.write(false);
    wait();
    do {
      wait();
    } while(!data_v.read());
    int d = data.read();
    at.add_entry(AddressTableEntry(a, d));
    wait();
    data_f.write(true);
    do {
      wait();
    } while(data_v.read());
  }
  else {
    wait();
    return;
  }
}

// l2cache functions
void l2cache::receive() {
  while(true) {
    for (int i =0; i < L2CACHECORENUM; i++) {
      if (core_memory_request[i].read()) {
        core_memory_accept[i].write(true);
        do {
          wait();
        } while(!addr_v.read());
        bool read = rw.read();
        if (read) {
          PRINTL("read data from l2cache", "");
          read_data();
        }
        else {
          write_data();
        }
        core_memory_accept[i].write(false);
      }
      wait();
    }
    wait();
  }
}

void l2cache::read_data() {
  if (addr_v.read()) {
    mem_addr a = addr.read();
    addr_f.write(true);
    PRINTL("Reading data for address %d from l2cache", a);
    do {
      wait();
    } while(addr_v.read());
    addr_f.write(false);
    wait();
    AddressTableEntry ate = at.get_entry(AddressTableEntry(a));
    if (ate.get_addr() == 0) {
      // no address found
      PRINTL("Address %d not found", a);
      memory_segment_addr.write(a);
      memory_segment_addr_v.write(true);
      memory_segment_rw.write(true);
      do {
        wait();
      } while(!memory_segment_addr_f.read());
      memory_segment_addr_v.write(false);
      memory_segment_data_f.write(false);
      do {
        //PRINTL("l2cache wait memory segment data v", "");
        wait();
      } while(!memory_segment_data_v.read());
      data_v.write(true);
      int d = memory_segment_data.read();
      at.add_entry(AddressTableEntry(a, d));
      data.write(d);
      memory_segment_data_f.write(true);
      do {
        wait();
      }while(!data_f.read());
      data_v.write(false);
      wait();
    }
    else {
      int d = ate.get_data();
      PRINTL("Address %d found with data %d", a, d);
      for (int i = 0; i < L2CACHEDELAY-1; i++) {
        wait();
      }
      data_v.write(true);
      data.write(d);
      wait();
      do {
        wait();
      } while(!data_f.read());
      data_v.write(false);
      wait();
    }
  }
  else {
    wait();
    return;
  }
}

void l2cache::write_data() {
  if (addr_v.read()) {
    mem_addr a = addr.read();
    addr_f.write(true);
    PRINTL("Writing data for address %d", a);
    do {
      wait();
    } while(addr_v.read());
    addr_f.write(false);
    wait();
    do {
      wait();
    } while(!data_v.read());
    int d = data.read();
    at.add_entry(AddressTableEntry(a, d));
    wait();
    data_f.write(true);
    do {
      wait();
    } while(data_v.read());
  }
  else {
    wait();
    return;
  }
}
