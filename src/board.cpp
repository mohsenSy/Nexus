#include <systemc.h>
#include <board.h>

#include <utils.h>

void board::receiveTask() {
  rdy.write(true);
  t_in_f.write(false);
  while(true) {
    task t = t_in.read();
    t_in_f.write(false);
    if(t_in_v.read() == true && t != previous_task) {
      previous_task = t;
      while(num_tasks == TASK_NUM) {
        // The task buffer is full and cannot receive new tasks
        rdy.write(false);
        wait();
      }
      rdy.write(true);
      taskFifo.write(t);
      num_tasks++;
      t_in_f.write(true);
      PRINTL("Board new task with id %d", t.id);
    }
    wait();
  }
}

void board::sendTask() {
  while (true) {
    // Read a task from FIFO
    task t;
    while (!taskFifo.nb_read(t)) {
      // No tasks
      //PRINTL("No tasks to send","");
      wait();
    }
    PRINTL("Read a task with id %d from buffer", t.id);

    // Loop through the cores to find a ready one
    int i = 0;
    while( rdy_sigs[i] != true) {
      PRINTL("waiting for core after %d", i);
      wait();
      if (++i == CORE_NUM) {
        i = 0;
      }
    }
    PRINTL("Sending task with id %d to core %d", t.id, i);

    // Send task to the chosen core
    t_in_sigs[i] = t;
    t_in_v_sigs[i] = true;

    while (t_in_f_sigs[i] != true) {
      // Wait untl the task is read by the core unit
      PRINTL("Wait until task %d is read by core %d", t.id, i);
      wait();
    }
    t_in_v_sigs[i] = false;
    num_tasks--;
    wait();
  }
}
