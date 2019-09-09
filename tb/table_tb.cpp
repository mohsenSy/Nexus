
#include <table.h>
#include <iostream>
#include <systemc>

class entry {
  public:
    int id;
    int data;
    entry(int id = 0, int data = 0) : id(id), data(data) {}
    friend ostream& operator <<(ostream& out, const entry& e) {
      out << "id: " << e.id << " data: " << e.data;
      return out;
    }
    bool operator ==(const entry& e) {
      return id == e.id;
    }
};

int sc_main(int argc, char **argv) {

  Table<entry> t = Table<entry>(12);

  t.add_entry(entry());
  t.add_entry(entry(1));
  t.add_entry(entry(2,3));
  t.dump();

  return 0;
}
