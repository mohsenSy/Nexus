#include <core.h>
#include <utils.h>


void core::handle_finished(void) {
  while(true) {
    t_out_v.write(false);
    if(t_out_v_sig == true) {
      task t = t_out_sig;
      t_out_f_sig = true;
      PRINTL("task finished %d",t.id);
      num_tasks--;
      //finished.write(true);
      t_out.write(t);
      t_out_v.write(true);
      //rdy.write(true);
      t_out_f_sig = true;
      wait();
      t_out_f_sig = false;
      // This will be added when Nexus is implemented, right now no other unit reads the finished task.
      /*while(t_out_f.read() == false) {
        // Wait for the finished task to be read by the other unit
        wait();
      }*/
    }
    wait();
  }
}

void core::prepare(void) {
  /*
    This function prepares the current task for execution by sending it to execute module
  */
  // TODO: Make the prepare function suitable for BUFFER_DEPTH greater than 2
  rdy.write(true);
  t_in_f.write(false);

  while (true) {
    if(t_in_v.read()) {
      task t = t_in.read();
      if (previous_task != t) {
        PRINTL("receive task with id %d", t.id);
        previous_task = t;
        t_in_f.write(true);
        wait();
        //PRINTL("preparing task %d", t.id);
        // If the buffer is full wait until a task finishes
        while(num_tasks == BUFFER_DEPTH - 1) {
          //PRINTL("task %d is waiting for buffer", t.id);
          rdy.write(false);
          if ( num_tasks < BUFFER_DEPTH - 1 ) {
            PRINTL("Setting rdy to true", "");
            rdy.write(true);
            wait();
            break;
          }
          wait();
        }
        /*if(++num_tasks == BUFFER_DEPTH) {
          // Cannot receive any more tasks
          //PRINTL("Setting rdy to false","");
          rdy.write(false);
          wait();
        }*/
        ts[num_tasks++] = t;
        t_in_f.write(false);
        rdy.write(true);
        wait();
        //rdy.write(true);
        // Wait until the execution unit finishes current task
        while (rdy_sig == false) {
          wait();
        }
        PRINTL("Ready to send task %d to execution unit", t.id);
        // Send task to execution unit
        t_in_sig = t;
        t_in_v_sig = true;
        wait();

        while (t_in_f_sig != true) {
          // Wait until task is read by execution unit
          //PRINTL("Waiting for execution unit for task with id %d", t.id);
          wait();
        }
        //PRINTL("Ready to send task %d to execution unit", t.id);
        t_in_v_sig = false;
      }
    }
    wait();
  }
}
