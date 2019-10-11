#include <systemc.h>
#include <board1.h>

#include <utils.h>
#include <stats.h>

void board::send_finished_nexus(task t) {
  if (!nex) {
    return;
  }
  t_f_in_v_sig = true;
  t_f_in_sig = t;
  PRINTL("board::send_finished_nexus: Sending finished task %d to Nexus", t.id);
  do {
    wait();
  }while(!t_f_in_f_sig);
  PRINTL("board::send_finished_nexus: Finished task %d was sent to nexus", t.id);
  t_f_in_v_sig = false;
  wait();
}

void board::read_finished() {
  while(true) {
    for (int i = 0; i < CORE_NUM; i++) {
      if (t_out_v_sigs[i].read()) {
        task t = t_out_sigs[i].read();
        PRINTL("board::read_finished: finished task %d from core %d", t.id, i);
        t_out_f_sigs[i].write(true);
        send_finished_nexus(t);
        wait();
        t_out_f_sigs[i].write(false);
      }
      wait();
    }
    wait();
  }
}

void board::receiveTask() {
  rdy.write(true);
  t_in_f.write(false);
  while(true) {
    task t = t_in.read();
    t_in_f.write(false);
    if(t_in_v.read() == true && t != previous_task) {
      previous_task = t;
      PRINTL("board::receiveTask: Recived new task %d", t.id);
      while(!taskFifo.nb_write(t)) {
        // The task buffer is full and cannot receive new tasks
        rdy.write(false);
        //PRINTL("Board task fifo is full", "");
        print_once(0, "Board task fifo is full cannot receive task %d", t.id);
        wait();
        Stats::inc_board_buffer_wait_cycles();
      }
      reset_printe_once(0);
      PRINTL("board::receiveTask: Task %d was written to task fifo", t.id);
      rdy.write(true);
      t_in_f.write(true);
    }
    wait();
  }
}

void board::send_task_nexus(task t) {
  // Make sure the nexus unit is ready to receive a new task
  while(rdy_sig == false) {
    wait();
  }
  t_in_v_sig = true;
  t_in_sig = t;
  PRINTL("board::send_task_nexus: Sending task %d to nexus", t.id);
  wait();
  // Make sure the task is read by nexus
  while (t_in_f_sig != true) {
    print_once(1, "Cannot send task %d to nexus", t.id);
    wait();
  }
  reset_printe_once(1);
  PRINTL("board::send_task_nexus: Task %d was sent to nexus", t.id);
  t_in_v_sig = false;
  wait();
}

void board::send_task_core(task t) {
  int i = 0;
  // Loop through the cores to find a ready one
  while( rdy_sigs[i] != true) {
    wait();
    PRINTL("board::send_task_core: Core %d is not ready", i);
    Stats::inc_core_wait_cycles();
    if (++i == CORE_NUM) {
      i = 0;
    }
  }
  PRINTL("board::send_task_core: Sending task with id %d to core %d", t.id, i);

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
  PRINTL("board::send_task_core: Task %d was read by core %d", t.id, i);
  t_in_v_sigs[i] = false;
}

void board::send_ready_tasks() {
  while(true) {
    task t;
    while(!ready_queue.nb_read(t)) {
      wait();
    }
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
    PRINTL("board::read_ready_tasks: received ready task %d in board", t.id);
    while(!ready_queue.nb_write(t)) {
      wait();
    }
    while(t_ready_out_v_sig.read()) {
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
      wait();
    }
    PRINTL("board::sendTask: Read a task with id %d from buffer", t.id);
    // send task to nexus
    if (nex) {
      send_task_nexus(t);
      PRINTL("board::sendTask: Sent task %d to nexus", t.id);
    }
    else {
      send_task_core(t);
    }
    wait();
  }
}
