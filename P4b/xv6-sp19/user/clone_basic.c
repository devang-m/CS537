/*
 * test for basic functionailty of clone(), with no cleanup
 * Authors:
 * - Varun Naik, Spring 2018
 */
#include "types.h"
#include "stat.h"
#include "user.h"

#define PGSIZE 0x1000

int ppid = 0;
volatile int global = 0;
char *stack = 0;

void
func(void *arg1, void *arg2)
{
  // The test case passes as soon as this message is printed
  printf(1, "PASSED TEST!\n");
}

void
func1(void *arg1, void *arg2)
{
  // The test case passes as soon as this message is printed
  printf(1, "PASSED TEST!\n");
}

int
main(int argc, char *argv[])
{
  int pid;
  int arg1 = 0xABCD, arg2 = 0xABCD;
  void *st;
  // Expand address space for stack
  stack = sbrk(2*PGSIZE);
  stack += PGSIZE;

  pid = clone(&func1, &arg1, &arg2, stack);
  pid = join(&st);

  ++pid;
  // Sleep, so that the test passes before the parent terminates
  sleep(10);

  exit();
}
