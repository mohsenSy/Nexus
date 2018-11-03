#include <systemc.h>
#include <nexus.h>

struct entry {
  int id;
  int data;
};

int sc_main(int argc, char **argv) {

  Table *t = new Table(12);

  struct entry e;
  e.id = 1;
  e.data = 90;

  t->add_entry((void *)&e, 1);
  t->add_entry((void *)&e, 2);
  t->add_entry((void *)&e, 3);
  t->print_entries();
  t->delete_entry(1);
  t->add_entry((void *)&e, 1);
  t->print_entries();

  return 0;
}
