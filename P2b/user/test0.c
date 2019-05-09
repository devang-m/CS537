#include "types.h"
#include "stat.h"
#include "user.h"
#include "pstat.h"

int
main(int argc, char *argv[])
{
    struct pstat st;

    if(argc != 2){
        printf(1, "usage: mytest counter");
        exit();
    }

    int i, x, l, j, child = 0;
    int mypid = getpid();
    int rc = fork();
    if(rc == 0) {
      child = -1;
      for(i = 1; i < atoi(argv[1]); i++){
        x = x + i;
      }
    }
    else {
      child = rc;
      rc = (int) wait(); 
      for(i = 1; i < atoi(argv[1]); i++){
        x = x + i;
      }
     
    }
    getpinfo(&st);
    mypid = getpid();
    if (child == -1)
      printf(1, "Child Process\n");
    else
      printf(1, "Parent's PID is %d Child's PID is %d\n", mypid, child);
    for (j = 0; j < NPROC; j++) {
        if (st.inuse[j] && st.pid[j] >= 3 && st.pid[j] == mypid) {
            for (l = 3; l >= 0; l--) {
	      printf(1, "process id: %d level:%d \t ticks-used:%d\n", getpid(), l, st.ticks[j][l]);
            }
        }
    }
    
    exit();
    return 0;
}
