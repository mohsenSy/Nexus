#include <core.h>
#include <utils.h>
#include <mutex>

sc_mutex memory_mutex;

void core::handle_finished(void) {
  while(true) {
    t_out_v.write(false);
    if(t_out_v_sig == true) {
      task t = t_out_sig;
      t_out_f_sig = true;
      PRINTL("task finished %d",t.id);
      this->m.lock();
      this->num_tasks--;
      this->m.unlock();
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

void core::fetch_input(mem_addr addr) {
  // Wait until memory controller is ready
  while(memory_rdy.read() != true) {
    wait();
  }
  memory_mutex.lock();
  // Send address to memory controller
  memory_addr.write(addr);
  memory_addr_v.write(true);
  wait();
  // Wait until address is read by memory controller
  while(memory_addr_f.read() == false) {
    wait();
  }
  memory_mutex.unlock();
}

void core::send_task(void) {
  while (true) {
    task t;
    while(!taskFifo.nb_read(t)) {
      wait();
    }
    int num_inputs = t.input_args;
    // fetch input args
    PRINTL("fetching inputs for %d, %d", t.id, num_inputs);
    for (int i = 0; i < num_inputs; i++) {
      fetch_input(t.in_args[i]);
    }
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

void core::prepare(void) {
  /*
    This function prepares the current task for execution by sending it to execute module
  */
  // TODO: Make the prepare function suitable for BUFFER_DEPTH greater than 2
  rdy.write(true);
  t_in_f.write(false);

  while (true) {
    if (num_tasks == BUFFER_DEPTH) {
      wait();
      continue;
    }
    rdy.write(true);
    if(t_in_v.read()) {
      task t = t_in.read();
      if (previous_task != t) {
        previous_task = t;
        //PRINTL("preparing task %d", t.id);
        // If the buffer is full wait until a task finishes
        PRINTL("BUFFER_DEPTH is %d, num_tasks is %d", BUFFER_DEPTH, this->num_tasks);
        while(this->num_tasks == BUFFER_DEPTH) {
          rdy.write(false);
          PRINTL("RDY is false", "");
          wait();
        }
        /*while(!taskFifo.nb_write(t)) {
          rdy.write(false);
          PRINTL("RDY is false", "");
          wait();
        }*/
        taskFifo.write(t);
        PRINTL("receive task with id %d", t.id);
        this->m.lock();
        this->num_tasks++;
        this->m.unlock();
        if (num_tasks == BUFFER_DEPTH) {
          rdy.write(false);
        }
        t_in_f.write(true);
        wait();
        t_in_f.write(false);
        wait();
        //rdy.write(true);
      }
    }
    wait();
  }
}
