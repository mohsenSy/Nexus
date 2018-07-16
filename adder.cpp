
#include "systemc.h"
#define WIDTH  4

SC_MODULE(adder) {
  sc_in<sc_uint<WIDTH> > a, b;
  sc_out<sc_uint<WIDTH> > sum;

  void do_add() {
    sum.write(a.read() + b.read());
  }
  SC_CTOR(adder)       {
    SC_METHOD(do_add);
    sensitive << a << b;
  }
};

int sc_main(int argc, char **argv) {

  adder *c = new adder();

  return 0;
}
