#include <execute.h>
#include <utils.h>

void execute::do_execute() {

  // Initialization
  // Ready to receive new task
  rdy.write(true);
  // Did not finish reading new task
  t_in_f.write(false);
  // Task output is not valid
  t_out_v.write(false);

  while (true) {
    // Check if task input is valid?
    if (t_in_v.read()) {
        task t = t_in.read();
        // Read new task and make sure it is a new one
        if (previous_task != t) {
          // Cannot receive new task
          rdy.write(false);
          //PRINTL("received task with id %d", t.id);
          // Finished reading new task
          t_in_f.write(true);
          previous_task = t;
          wait();
          // Get delay from task - Task Execution is modelled as delay here.
          //PRINTL("Delay is %d", t.delay);
          PRINTL("Started executing task %d", t.id);
          for (int i = 0; i < t.delay ; i++) {
            //PRINTL("executing task %d", t.id);
            wait();
          }
          // Write finished task to output port and set ouput as valid
          t_out.write(t);
          t_out_v.write(true);
          // Did not finish reading new task
          t_in_f.write(false);
          PRINTL("finished task %d", t.id);
          do {
            // Wait until task is read by core unit
            wait();
          } while(t_out_f.read() == false);
          // Invalidate task output
          t_out_v.write(false);
          // ready to receive new task
          rdy.write(true);
        }
    }
    wait();
  }
}
