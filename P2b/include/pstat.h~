#ifndef _PSTAT_H_
#define _PSTAT_H_

#include "param.h"

struct pstat {
  int inuse[NPROC]; // whether this slot of the process table is in use (1 or 0)
  int pid[NPROC];   // PID of each process
  int priority[NPROC];  // current priority level of each process (0-3)
  enum procstate state[NPROC];  // current state (e.g., SLEEPING or RUNNABLE) of each process
  int ticks[NPROC][NLAYER];  // number of ticks each process has accumulated at each of 4 priorities
  int wait_ticks[NPROC][NLAYER]; // number of ticks each process has waited before being scheduled
};

#endif // _PSTAT_H_
