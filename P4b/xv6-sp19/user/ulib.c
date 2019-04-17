#include "types.h"
#include "stat.h"
#include "fcntl.h"
#include "user.h"
#include "x86.h"

#define PGSIZE 0x1000
void* userStackAddress[64];
int valueInArray[64];

char*
strcpy(char *s, char *t)
{
  char *os;

  os = s;
  while((*s++ = *t++) != 0)
    ;
  return os;
}

int
strcmp(const char *p, const char *q)
{
  while(*p && *p == *q)
    p++, q++;
  return (uchar)*p - (uchar)*q;
}

uint
strlen(char *s)
{
  int n;

  for(n = 0; s[n]; n++)
    ;
  return n;
}

void*
memset(void *dst, int c, uint n)
{
  stosb(dst, c, n);
  return dst;
}

char*
strchr(const char *s, char c)
{
  for(; *s; s++)
    if(*s == c)
      return (char*)s;
  return 0;
}

char*
gets(char *buf, int max)
{
  int i, cc;
  char c;

  for(i=0; i+1 < max; ){
    cc = read(0, &c, 1);
    if(cc < 1)
      break;
    buf[i++] = c;
    if(c == '\n' || c == '\r')
      break;
  }
  buf[i] = '\0';
  return buf;
}

int
stat(char *n, struct stat *st)
{
  int fd;
  int r;

  fd = open(n, O_RDONLY);
  if(fd < 0)
    return -1;
  r = fstat(fd, st);
  close(fd);
  return r;
}

int
atoi(const char *s)
{
  int n;

  n = 0;
  while('0' <= *s && *s <= '9')
    n = n*10 + *s++ - '0';
  return n;
}

void*
memmove(void *vdst, void *vsrc, int n)
{
  char *dst, *src;
  
  dst = vdst;
  src = vsrc;
  while(n-- > 0)
    *dst++ = *src++;
  return vdst;
}

int 
thread_create(void (*start_routine)(void *, void *), void *arg1, void *arg2)
{
  void* stackAddr, *stackPassed;
  stackAddr = malloc(2*PGSIZE);
  // malloc did not work
  if(stackAddr == 0) {
    return -1;
  }
  //To make the stack page aligned
  int extraSpace = ((int)(stackAddr))%PGSIZE;
  stackPassed = (stackAddr) + PGSIZE - extraSpace;
  int childPid = clone(start_routine, arg1, arg2, stackPassed);
  if (childPid != -1) {
    for(int i=0; i<64; i++ ) {
      if(userStackAddress[i] == NULL) {
        userStackAddress[i] = stackAddr;
        valueInArray[i] = childPid;
        break;
      }
    }
  }
  else {
    free(stackAddr);
  }
  return childPid;
}

int
thread_join()
{
  void *stack;
  int childPid;
  //Need to free stack
  childPid = join(&stack);
  if (childPid != -1) {
    for(int i=0; i<64; i++) {
      if(valueInArray[i] == childPid) {
        free(userStackAddress[i]);
        valueInArray[i] = -1;
        userStackAddress[i] = NULL;
      }
    }
  }
  return childPid;
}

void lock_init(lock_t *lock) {
  lock->ticket = 0;
  lock->turn = 0;
}

void lock_acquire(lock_t *lock) {
  int myturn = fadd(&lock->ticket, 1);
  while (lock->turn != myturn);
}

void lock_release(lock_t *lock) {
  fadd(&lock->turn, 1);
}
