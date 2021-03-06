#ifndef __STATS_H
#define __STATS_H
#pragma once
#include <systemc.h>
#include <mutex>

class Stats {
  private:
    // Core related stats
    static inline long core_wait_cycles;
    static inline sc_mutex core_wait_cycles_mutex;
    static inline long core_finished_tasks_num;
    static inline sc_mutex core_finished_tasks_num_mutex;
    // Execute related stats
    static inline long execute_cycles;
    static inline sc_mutex execute_cycles_mutex;
    // Memory related stats
    static inline long memory_cycles;
    static inline sc_mutex memory_cycles_mutex;
    // Board related stats
    static inline long board_buffer_wait_cycles;
    static inline sc_mutex board_buffer_wait_cycles_mutex;
  public:
    // Core related
    void static inc_core_finished_tasks_num();
    void static print_core_finished_tasks_num();
    void static inc_core_wait_cycles();
    void static print_core_wait_cycles();
    // Execute related
    void static inc_execute_cycles();
    void static print_execute_cycles();
    // Memory related
    void static inc_memory_cycles();
    void static print_memory_cycles();
    // Board related
    void static inc_board_buffer_wait_cycles();
    void static print_board_buffer_wait_cycles();
};

#endif
