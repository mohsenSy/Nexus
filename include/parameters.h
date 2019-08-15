#ifndef _PARAMETERS_H
#define _PARAMETERS_H

// Core parameters
#define BUFFER_DEPTH 2 // Number of buffered tasks at the core level

// Memory Controller parameters
#define MEM_FETCH_TIME 10 // Number of cycles required to read from memory

// Board parameters
#define CORE_NUM 10 // Number of simulated cores in the System Board
#define TASK_NUM 101 // Number of tasks buffered in system board for execution
#define READY_QUEUE_NUM 10

// Nexus 1 parameters
#define NEXUS1_TASK_NUM 24
#define NEXUS1_IN_BUFFER_DEPTH 4
#define NEXUS1_READY_QUEUE_DEPTH 12
#define NEXUS1_TASK_TABLE_SIZE 12
#define NEXUS1_KICK_OFF_LIST_SIZE 6
#define NEXUS1_PRODUCERS_TABLE_SIZE 24
#define NEXUS1_CONSUMERS_TABLE_SIZE 24

// Nexus 2 parameters
#define NEXUS2_TASK_NUM 24
#define NEXUS2_TDS_BUFFER_DEPTH 4
#define NEXUS2_NEW_TASKS_NUM 10
#define NEXUS2_DEPS_TABLE_SIZE 128

#endif
