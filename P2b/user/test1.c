#include "types.h"
#include "stat.h"
#include "user.h"
#include "pstat.h"
#define check(exp, msg) if(exp) {} else {       \
  printf(1, "%s:%d check (" #exp ") failed: %s\n", __FILE__, __LINE__, msg); \
  exit();}

int pow2[] = {80000000, 32, 16, 8};

int workload(int n) {
  int i, j = 0;
  for(i = 0; i < n; i++)
    j += i * j + 1;
  return j;
}

int
main(int argc, char *argv[])
{
  struct pstat st;

  sleep(10);

  int i, j, k, count = 0;
  for (i = 0; i <= 60; i++) {
    if (fork() == 0) {
      workload(4000000 * (i + 1));
      if (i == NPROC - 4) {
        sleep(100);
        check(getpinfo(&st) == 0, "getpinfo");

        // See what's going on...
         
        for(k = 0; k < NPROC; k++) {
          if (st.inuse[k]) {
            int m;
            printf(1, "pid: %d\n", st.pid[k]);
            if (st.pid[k] > 3) {
                check(st.ticks[k][3] > 0, "Every process at the highest level should use at least 1 timer tick");
            }
            for (m = 3; m >= 0; m--) {
              printf(1, "\t level %d ticks used %d\n", m, st.ticks[k][m]);
            }
          }
        }
        
        

        for(k = 0; k < NPROC; k++) {
          if (st.inuse[k]) {
            count++;
            check(st.priority[k] <= 3 && st.priority[k] >= 0, "Priority should be 3, 2, 1 or 0");
            for (j = 3; j > st.priority[k]; j--) {
              if (st.ticks[k][j] != pow2[j]) {
                printf(1, "#ticks at this level should be %d, \
                    when the priority of the process is %d. But got %d\n", 
                    pow2[j], st.priority[k], st.ticks[k][j]);
                exit();
              }
            }
            if (st.ticks[k][j] > pow2[j]) {
              printf(1, "#ticks at level %d is %d, which exceeds the maximum #ticks %d allowed\n", j, st.ticks[k][j], pow2[j]);
              exit();
            }
          }
        }
        check(count == NPROC, "Should have 64 processes currently in used in the process table.");
        printf(1, "TEST PASSED");
      }
    } else {
      wait();
      break;
    }
  }

  exit();
}
