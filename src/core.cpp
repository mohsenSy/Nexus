#include <core.h>
#include <utils.h>
#include <mutex>

#include <stats.h>

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
      std::string n = name();
      std::string nn = n.append(".count");
      LOG(nn, "Finished task %d", t.id);
      Stats::inc_core_finished_tasks_num();
      t_out_f_sig = false;
      dec_tasks_num();
      t_out_v.write(true);
      t_out.write(t);
      while(t_out_f.read() == false) {
        // Wait for the finished task to be read by the other unit
        //PRINTL("Waiting for task %d to be read by the board", t.id);
        wait();
      }
      t_out_v.write(false);
    }
    wait();
  }
}

void core::fetch_input(mem_addr addr) {
  memory_request.write(true);
  while(memory_accept == false) {
    Stats::inc_memory_cycles();
    wait();
  }
  memory_addr.write(addr);
  memory_addr_v.write(true);
  while(memory_addr_f == false) {
    Stats::inc_memory_cycles();
    wait();
  }
  while(memory_data_rdy.read() == false) {
    Stats::inc_memory_cycles();
    wait();
  }
  memory_request.write(false);
  Stats::inc_memory_cycles();
  wait();
}

void core::send_task(void) {
  while (true) {
    task t;
    while(!readTask(t)) {
      wait();
    }
    int num_inputs = t.input_args;
    // fetch input args
    LOG(name(), "fetching inputs for %d", t.id);
    PRINTL("fetching inputs for %d, %d", t.id, num_inputs);
    for (int i = 0; i < num_inputs; i++) {
      fetch_input(t.in_args[i]);
    }
    LOG(name(), "Done fetching inputs for %d", t.id);
    // Wait until the execution unit finishes current task
    while (rdy_sig == false) {
      wait();
    }
    PRINTL("Ready to send task %d to execution unit", t.id);
    // Send task to execution unit
    t_in_sig = t;
    t_in_v_sig = true;
    wait();
    LOG(name(), "Sent task %d to execution", t.id);

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
        LOG(name(), "Task %d received", t.id);
        while (!addTask(t)) {
          rdy.write(false);
          wait();
        }
        inc_tasks_num();
        LOG(name(), "Task %d ready", t.id);
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
