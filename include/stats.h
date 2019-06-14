#ifndef __STATS_H
#define __STATS_H
#pragma once
#include <systemc.h>
#include <mutex>

class Stats {
  private:
    static inline long core_wait_cycles;
    static inline sc_mutex core_wait_cycles_mutex;
    static inline long execute_cycles;
    static inline sc_mutex execute_cycles_mutex;
    static inline long memory_cycles;
    static inline sc_mutex memory_cycles_mutex;
  public:
    void static inc_core_wait_cycles();
    void static print_core_wait_cycles();
    void static inc_execute_cycles();
    void static print_execute_cycles();
    void static inc_memory_cycles();
    void static print_memory_cycles();
};

#endif
