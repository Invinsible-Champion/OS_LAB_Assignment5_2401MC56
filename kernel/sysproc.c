#include "types.h"
#include "x86.h"
#include "defs.h"
#include "date.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"
#include "spinlock.h"

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
  return myproc()->pid;
}

int
sys_sbrk(void)
{
  int addr;
  int n;

  if(argint(0, &n) < 0)
    return -1;
  addr = myproc()->sz;
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
    if(myproc()->killed){
      release(&tickslock);
      return -1;
    }
    sleep(&ticks, &tickslock);
  }
  release(&tickslock);
  return 0;
}

// return how many clock tick interrupts have occurred
// since start.
int
sys_uptime(void)
{
  uint xticks;

  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}

// --- Assignment 5: Semaphores Implementation ---
#define MAX_SEM 10
// Structure representing a counting semaphore
struct sem {
  struct spinlock lock; // Protects the semaphore's count to ensure atomicity
  int count;            // Current value of the semaphore
  int active;           // 1 if initialized/in-use, 0 otherwise
};

struct sem sems[MAX_SEM];

// Initialize all semaphores to inactive state during boot
void
seminit(void)
{
  int i;
  for(i = 0; i < MAX_SEM; i++) {
    initlock(&sems[i].lock, "semaphore");
    sems[i].active = 0;
  }
}

// Syscall to initialize a specific semaphore to a given count
int
sys_sem_init(void)
{
  int id, val;
  if(argint(0, &id) < 0 || argint(1, &val) < 0)
    return -1;
  if(id < 0 || id >= MAX_SEM)
    return -1;
  
  acquire(&sems[id].lock);
  sems[id].count = val;
  sems[id].active = 1;
  release(&sems[id].lock);
  return 0;
}

// Syscall to decrement (wait/P) a semaphore
int
sys_sem_down(void)
{
  int id;
  if(argint(0, &id) < 0)
    return -1;
  if(id < 0 || id >= MAX_SEM)
    return -1;
  
  acquire(&sems[id].lock);
  // Sleep while the count is 0, releasing the lock to allow others to run
  while(sems[id].count == 0) {
    sleep(&sems[id], &sems[id].lock);
  }
  sems[id].count--;
  release(&sems[id].lock);
  return 0;
}

// Syscall to increment (signal/V) a semaphore
int
sys_sem_up(void)
{
  int id;
  if(argint(0, &id) < 0)
    return -1;
  if(id < 0 || id >= MAX_SEM)
    return -1;
  
  acquire(&sems[id].lock);
  sems[id].count++;
  // Wake up any processes sleeping on this semaphore
  wakeup(&sems[id]);
  release(&sems[id].lock);
  return 0;
}
// ----------------------------------------------

extern void* shm_get(void);

// Wrapper syscall to call the kernel shm_get implementation
int
sys_shm_get(void)
{
  return (int)shm_get();
}
