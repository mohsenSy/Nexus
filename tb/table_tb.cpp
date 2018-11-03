#include <systemc.h>
#include <nexus.h>

struct entry {
  int id;
  int data;
};

int sc_main(int argc, char **argv) {

  Table *t = new Table(12);

  struct entry *e = new struct entry;
  e->id = 1;
  e->data = 90;

  t->add_entry((void *)e, 1);
  e = new struct entry;
  e->id = 2;
  e->data = 91;
  t->add_entry((void *)e, 2);

  struct entry* ee;

  ee = (struct entry*)t->get_entry(2);
  if (ee != NULL) {
    std::cout << ee->data << std::endl;
  }

  return 0;
}
