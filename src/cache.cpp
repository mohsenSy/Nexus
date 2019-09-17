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

void l1cache::read_data() {
  if (addr_v.read()) {
    mem_addr a = addr.read();
    addr_f.write(true);
    PRINTL("Reading data for address %d", a);
    do {
      wait();
    } while(addr_v.read());
    addr_f.write(false);
    wait();
    AddressTableEntry ate = at.get_entry(AddressTableEntry(a));
    if (ate.get_addr() == 0) {
      // no address found
      PRINTL("Address %d not found", a);
      // For testing only
      at.add_entry(AddressTableEntry(a));
      data_v.write(true);
      data.write(90);
      wait();
      do {
        wait();
      } while(!data_f.read());
      data_v.write(false);
      wait();
    }
    else {
      int d = ate.get_data();
      PRINTL("Address %d found with data %d", a, d);
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
