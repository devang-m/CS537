#include "types.h"
#include "x86.h"
#include "defs.h"
#include "param.h"
#include "mmu.h"
#include "proc.h"
#include "sysfunc.h"

extern void* shared_addr[3];

int
sys_fork(void)
{
  return fork();
}

int
sys_exit(void)
{
  exit();
  return 0;  // not reached
}

int
sys_wait(void)
{
  return wait();
}

int
sys_kill(void)
{
  int pid;

  if(argint(0, &pid) < 0)
    return -1;
  return kill(pid);
}

int
sys_getpid(void)
{
  return proc->pid;
}

int
sys_sbrk(void)
{
  int addr;
  int n;

  if(argint(0, &n) < 0)
    return -1;
  addr = proc->sz;
  if(growproc(n) < 0)
    return -1;
  return addr;
}

int
sys_sleep(void)
{
  int n;
  uint ticks0;
  
  if(argint(0, &n) < 0)
    return -1;
  acquire(&tickslock);
  ticks0 = ticks;
  while(ticks - ticks0 < n){
    if(proc->killed){
      release(&tickslock);
      return -1;
    }
    sleep(&ticks, &tickslock);
  }
  release(&tickslock);
  return 0;
}

// return how many clock tick interrupts have occurred
// since boot.
int
sys_uptime(void)
{
  uint xticks;
  
  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}

int
sys_shmget(void)
{
  int n;
  if(argint(0, &n) < 0)
    return -1;
  if(n<0 || n>2) {
    return 0;
  }
  int flag = 0;
  for(int i = 0; i < 3; i++) {
    if(proc->sharePageMap[i] == n) {
      flag = 1;
      return 4096*(i+1);
    }
  }
  if(flag == 0) {
    for(int i = 0; i < 3; i++) {
      if(proc->sharePageMap[i] == -1) {
        if(mapsharedpage(i, n) < 0)
          return -1;
        proc->sharePageMap[i] = n;
        return 4096*(i+1);
      }
    }
  }
  return 1;
}