#include <systemc.h>
#include <board.h>

#include <utils.h>
#include <stats.h>

void board::receiveTask() {
  rdy.write(true);
  t_in_f.write(false);
  while(true) {
    task t = t_in.read();
    t_in_f.write(false);
    if(t_in_v.read() == true && t != previous_task) {
      previous_task = t;
      while(!taskFifo.nb_write(t)) {
        // The task buffer is full and cannot receive new tasks
        rdy.write(false);
        wait();
        Stats::inc_board_buffer_wait_cycles();
      }
      rdy.write(true);
      t_in_f.write(true);
    }
    wait();
  }
}

void board::sendTask() {
  int i = 0;
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
    while( rdy_sigs[i] != true) {
      wait();
      PRINTL("Waiting for core after %d", i);
      Stats::inc_core_wait_cycles();
      if (++i == CORE_NUM) {
        i = 0;
      }
    }
    //std::cout << sc_time_stamp() << " : " << rdy_sigs[i] << std::endl;
    PRINTL("Sending task with id %d to core %d", t.id, i);

    // Send task to the chosen core
    t_in_sigs[i] = t;
    t_in_v_sigs[i] = true;
    wait();
    while (t_in_f_sigs[i] != true) {
      // Wait untl the task is read by the core unit
      //PRINTL("Wait until task %d is read by core %d", t.id, i);
      //Stats::inc_core_wait_cycles();
      wait();
    }
    t_in_v_sigs[i] = false;
    wait();
  }
}
