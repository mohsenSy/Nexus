#include <core.h>
#include <utils.h>
#include <mutex>

sc_mutex memory_mutex;

void core::handle_finished(void) {
  while(true) {
    t_out_v.write(false);
    if(t_out_v_sig == true) {
      task t = t_out_sig;
      //finished.write(true);
      t_out.write(t);
      t_out_v.write(true);
      //rdy.write(true);
      t_out_f_sig = true;
      wait();
      t_out_f_sig = false;
      dec_tasks_num();
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
    mem_cycles+=2;
    wait();
  }
  memory_mutex.lock();
  // Send address to memory controller
  memory_addr.write(addr);
  memory_addr_v.write(true);
  mem_cycles+=2;
  wait();
  // Wait until address data is ready
  while(memory_addr_rdy.read() == false) {
    mem_cycles+=2;
    wait();
  }
  memory_addr_v = false;
  wait();
  memory_mutex.unlock();
}

void core::send_task(void) {
  while (true) {
    task t;
    while(!readTask(t)) {
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
    while(get_tasks_num() == BUFFER_DEPTH) {
      rdy.write(false);
      wait();
    }
    if(t_in_v.read()) {
      task t = t_in.read();
      if (previous_task != t) {
        previous_task = t;
        //PRINTL("preparing task %d", t.id);
        // If the buffer is full wait until a task finishes
        // PRINTL("BUFFER_DEPTH is %d, num_tasks is %d", BUFFER_DEPTH, this->num_tasks);
        /*while(this->num_tasks == BUFFER_DEPTH) {
          rdy.write(false);
          wait();
        }*/
        /*while(!taskFifo.nb_write(t)) {
          rdy.write(false);
          PRINTL("RDY is false", "");
          wait();
        }*/
        while (!addTask(t)) {
          rdy.write(false);
          wait();
        }
        inc_tasks_num();
        //PRINTL("receive task with id %d", t.id);
        PRINTL("Num of tasks is %d", get_tasks_num());
        if (get_tasks_num() == BUFFER_DEPTH) {
          rdy.write(false);
          wait();
        }
        t_in_f.write(true);
        wait();
        t_in_f.write(false);
        wait();
        while(get_tasks_num() == BUFFER_DEPTH) {
          rdy.write(false);
          wait();
        }
      }
    }
    rdy.write(true);
    wait();
  }
}

bool core::addTask(task &t) {
  task_fifo_mutex.lock();
  bool ret = taskFifo.nb_write(t);
  task_fifo_mutex.unlock();
  return ret;
}

bool core::readTask(task &t) {
  task_fifo_mutex.lock();
  bool ret = taskFifo.nb_read(t);
  task_fifo_mutex.unlock();
  return ret;
}

int core::num_free() {
  task_fifo_mutex.lock();
  int ret = taskFifo.num_free();
  task_fifo_mutex.unlock();
  return ret;
}

void core::inc_tasks_num() {
  tasks_num_mutex.lock();
  num_tasks++;
  tasks_num_mutex.unlock();
}

void core::dec_tasks_num() {
  tasks_num_mutex.lock();
  num_tasks--;
  tasks_num_mutex.unlock();
}
int core::get_tasks_num() {
  tasks_num_mutex.lock();
  int num = num_tasks;
  tasks_num_mutex.unlock();
  return num;
}
