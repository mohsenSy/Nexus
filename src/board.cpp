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

void board::send_task_nexus(task t) {
  // Make sure the nexus unit is ready to receive a new task
  while(rdy_sig == false) {
    cout << "Waiting for RDY" << std::endl;
    wait();
  }
  //std::cout << "Sending task " << t.id << std::endl;
  t_in_v_sig = true;
  t_in_sig = t;
  wait();
  // Make sure the task is read by core unit
  while (t_in_f_sig != true) {
    wait();
  }
  //t_in_v_sig = false;
  wait();
}

void board::send_task_core(task t) {
  int i = 0;
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
}

void board::send_ready_tasks() {
  while(true) {
    task t;
    while(!ready_queue.nb_read(t)) {
      wait();
    }
    PRINTL("Sending task %d to ready core", t.id);
    send_task_core(t);
    wait();
  }
}

void board::read_ready_tasks() {
  while(true) {
    while(!t_ready_out_v_sig.read()) {
      wait();
    }
    task t = t_ready_out_sig.read();
    t_ready_out_f_sig.write(true);
    while(!ready_queue.nb_write(t)) {
      wait();
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
    // send task to nexus
    if (nex) {
      send_task_nexus(t);
    }
    else {
      send_task_core(t);
    }
    wait();
  }
}
