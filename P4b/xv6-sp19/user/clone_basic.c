/*
 * create and join many threads
 * Authors:
 * - Varun Naik, Spring 2018
 * - Inspired by a test case from Spring 2016, last modified by Akshay Uttamani
 */
#include "types.h"
#include "stat.h"
#include "user.h"

#define PGSIZE 0x1000
#define MIN_THREADS 50
#define MAX_THREADS 64
#define check(exp, msg) if(exp) {} else {\
  printf(1, "%s:%d check (" #exp ") failed: %s\n", __FILE__, __LINE__, msg);\
  printf(1, "TEST FAILED\n");\
  kill(ppid);\
  exit();}

int ppid = 0;
volatile int global = 0;
volatile int lastpid = 0;

static inline uint rdtsc() {
  uint lo, hi;
  asm("rdtsc" : "=a" (lo), "=d" (hi));
  return lo;
}

void
func1(void *arg1, void *arg2)
{
  volatile int pid;

  // Sleep, so that (most of) the child thread runs after the main thread exits
  sleep(100);

  // Make sure the scheduler is sane
  check(global == 1, "global is incorrect");

  pid = getpid();
  check(ppid < pid && pid <= lastpid, "getpid() returned the wrong pid");
  sleep(1);
  check(pid == getpid(), "pid was updated by another thread");

  exit();

  check(0, "Continued after exit");
}

void
func2(void *arg1, void *arg2)
{
  check(global == 1, "global is incorrect");
  --global;

  exit();

  check(0, "Continued after exit");
}

int
fill_ptable(void)
{
  int num_threads, pid, status, i;

  printf(1, "Creating child threads...\n");
  for (i = 0; i < MAX_THREADS; ++i) {
    pid = thread_create(&func1, NULL, NULL);
    if (pid != -1) {
      check(pid > lastpid, "thread_create() returned the wrong pid");
      lastpid = pid;
    } else {
      printf(1, "Created %d child threads\n", i);
      check(i >= MIN_THREADS, "Not enough threads created");
      global = 1;
      break;
    }
  }
  num_threads = i;
  check(i < MAX_THREADS, "Should not have created max threads");

  printf(1, "Joining all %d child threads...\n");
  for (i = 0; i < num_threads; ++i) {
    pid = thread_join();
    status = kill(pid);
    check(status == -1, "Child was still alive after thread_join()");
    check(ppid < pid && pid <= lastpid, "thread_join() returned the wrong pid");
  }

  printf(1, "All %d child threads joined\n", num_threads);

  return num_threads;
}

void
multiple_thread_create(void)
{
  int pid1, pid2, status, i;

  printf(1, "Creating and joining 10000 child threads...\n");
  for (i = 0; i < 1000; ++i) {
    ++global;
    pid1 = thread_create(&func2, NULL, NULL);
    check(pid1 > ppid, "thread_create() failed");
    pid2 = thread_join();
    status = kill(pid1);
    check(status == -1, "Child was still alive after thread_join()");
    check(pid1 == pid2, "thread_join() returned the wrong pid");
    check(global == 0, "global is incorrect");
  }
}

void
multiple_fork(void)
{
  int pid, i;

  printf(1, "Forking and joining 100 child processes...\n");
  for (i = 0; i < 100; ++i) {
    pid = fork();
    check(pid >= 0, "fork() failed");

    if (pid > 0) {
      // Parent process
      check(pid > ppid, "fork() failed");
      pid = wait();
      check(pid > ppid, "wait() failed");
      check(global == 0, "global is incorrect");

    } else {
      // Child process
      pid = getpid();
      check(pid > ppid, "fork() failed");
      ++global;
      exit();
      check(0, "Continued after exit");
    }
  }
}

int
main(int argc, char *argv[])
{
  int count1, count2;
  void *unused;

  ppid = getpid();
  check(ppid > 2, "getpid() failed");
  lastpid = ppid;
/*
  check(
    (uint)&argc < 3*PGSIZE,
    "Program uses too much memory, stack of main thread should be in first three pages"
  );
*/
  // With the given allocator, after this line, malloc() will (probably) not be
  // page aligned
  unused = malloc(rdtsc() % (PGSIZE-1) + 1);

  // Try to fill up process table
  count1 = fill_ptable();
  global = 0;
  count2 = fill_ptable();
  global = 0;
  check(count1 <= count2, "First round created more threads than second round");

  // Try to crash from a memory leak in thread_create() or thread_join()
  multiple_thread_create();
  global = 0;

  // Try to crash from a memory leak in fork() or wait()
  multiple_fork();

  free(unused);
  printf(1, "PASSED TEST!\n");
  exit();
}
