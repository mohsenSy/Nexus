#include <nexus1.h>
using namespace nexus1;
int sc_main(int args, char **argv) {

  /*sc_signal<task> t_sig;
  int delay = 10;

  task t;
  t.func_pointer = (void*)&delay;

  core* c = new core();

  c.t_in(t_sig);

  sc_start(20,SC_NS);

  t_sig = t;

  sc_stop();*/
  /*KickOfList list = KickOfList();
  task t;
  t.id = 1;
  std::cout << list.push(t) << std::endl;
  t.id = 9;
  std::cout << list.push(t) << std::endl;
  t.id = 10;
  std::cout << list.push(t) << std::endl;
  t.id = 11;
  std::cout << list.push(t) << std::endl;
  t.id = 1234;
  std::cout << "Size: " << list.get_size() << std::endl;
  std::cout << list.push(t) << std::endl;
  t.id = 567;
  std::cout << list.push(t) << std::endl;
  t.id = 342;
  std::cout << "Size: " << list.get_size() << std::endl;
  std::cout << list.push(t) << std::endl;
  list.print();
  list.pop();
  std::cout << "Empty? " << list.empty() << std::endl;
  std::cout << "Size: " << list.get_size() << std::endl;
  list.print();
  list.pop();
  list.pop();
  list.pop();
  std::cout << "Size: " << list.get_size() << std::endl;
  list.pop();
  std::cout << "Empty? " << list.empty() << std::endl;
  list.pop();
  list.pop();
  std::cout << "Size: " << list.get_size() << std::endl;
  std::cout << "Empty? " << list.empty() << std::endl;*/

  /*ProducersTable pt = ProducersTable(10);
  task t;
  t.id = 1;
  mem_addr addr = (mem_addr)12;
  pt.add_task(addr, t);
  t.id = 90;
  pt.add_task(addr, t);
  addr = (mem_addr)16;
  t.id = 2;
  pt.add_task(addr, t);
  addr = (mem_addr)15;
  t.id = 45;
  pt.add_task(addr, t);
  t.id = 123;
  pt.add_task(addr, t);
  pt.print();*/
  /*ConsumersTable ct = ConsumersTable(10);
  task t;
  t.id = 10;
  mem_addr addr = (mem_addr)11;
  ct.add_addr(addr);
  t.id = 11;
  ct.add_task(addr, t);
  t.id = 10;
  ct.add_task(addr, t);
  t.id = 111;
  addr = (mem_addr)23;
  ct.add_addr(addr);
  ct.add_addr(addr);
  ct.add_addr(addr);
  ct.add_task(addr, t);
  ct.print();*/

  return 0;
}
