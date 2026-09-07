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

// Semaphores
#define MAX_SEM 10
struct sem {
  struct spinlock lock;
  int count;
  int active;
};

struct sem sems[MAX_SEM];

void
seminit(void)
{
  int i;
  for(i = 0; i < MAX_SEM; i++) {
    initlock(&sems[i].lock, "semaphore");
    sems[i].active = 0;
  }
}

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

int
sys_sem_down(void)
{
  int id;
  if(argint(0, &id) < 0)
    return -1;
  if(id < 0 || id >= MAX_SEM)
    return -1;
  
  acquire(&sems[id].lock);
  while(sems[id].count == 0) {
    sleep(&sems[id], &sems[id].lock);
  }
  sems[id].count--;
  release(&sems[id].lock);
  return 0;
}

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
  wakeup(&sems[id]);
  release(&sems[id].lock);
  return 0;
}

extern void* shm_get(void);

int
sys_shm_get(void)
{
  return (int)shm_get();
}
