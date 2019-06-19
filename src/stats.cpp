#include <stats.h>
#include <systemc>
#include <mutex>

void Stats::inc_core_wait_cycles() {
  core_wait_cycles_mutex.lock();
  core_wait_cycles++;
  core_wait_cycles_mutex.unlock();
}
void Stats::print_core_wait_cycles() {
  core_wait_cycles_mutex.lock();
  std::cout << sc_time_stamp() << ": Core Wait Cycles: " << core_wait_cycles << std::endl;
  core_wait_cycles_mutex.unlock();
}

void Stats::inc_core_finished_tasks_num() {
  core_finished_tasks_num_mutex.lock();
  core_finished_tasks_num++;
  core_finished_tasks_num_mutex.unlock();
}
void Stats::print_core_finished_tasks_num() {
  core_finished_tasks_num_mutex.lock();
  std::cout << sc_time_stamp() << ": Number of finished tasks is " << core_finished_tasks_num << std::endl;
  core_finished_tasks_num_mutex.unlock();
}

void Stats::inc_execute_cycles() {
  execute_cycles_mutex.lock();
  execute_cycles++;
  execute_cycles_mutex.unlock();
}
void Stats::print_execute_cycles() {
  execute_cycles_mutex.lock();
  std::cout << sc_time_stamp() << ": Execute Cycles: " << execute_cycles << std::endl;
  execute_cycles_mutex.unlock();
}

void Stats::inc_memory_cycles() {
  memory_cycles_mutex.lock();
  memory_cycles++;
  memory_cycles_mutex.unlock();
}
void Stats::print_memory_cycles() {
  memory_cycles_mutex.lock();
  std::cout << sc_time_stamp() << ": Memory Cycles: " << memory_cycles << std::endl;
  memory_cycles_mutex.unlock();
}

void Stats::inc_board_buffer_wait_cycles() {
  board_buffer_wait_cycles_mutex.lock();
  board_buffer_wait_cycles++;
  board_buffer_wait_cycles_mutex.unlock();
}
void Stats::print_board_buffer_wait_cycles() {
  board_buffer_wait_cycles_mutex.lock();
  std::cout << sc_time_stamp() << ": Board Buffer Wait Cycles: " << board_buffer_wait_cycles << std::endl;
  board_buffer_wait_cycles_mutex.unlock();
}
