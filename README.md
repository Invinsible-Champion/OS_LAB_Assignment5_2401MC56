# Assignment 5: Process Synchronization in xv6

## Overview
This assignment implements process synchronization mechanisms in xv6 to solve classic concurrency problems: Peterson's Algorithm, Producer-Consumer, Readers-Writers, and Dining Philosophers.

## New System Calls

### Shared Memory (`shm_get`)
xv6 processes do not share memory by default as `fork()` creates a copy of the parent's memory space. To allow sharing, a new system call `shm_get()` was added.
- **Design:** `shm_get()` maps a single global physical page (`shm_page`) to a fixed virtual address (`0x40000000`) in the calling process's page table.
- **Usage:** Both the parent and the child must call `shm_get()`. The parent maps it, then forks. The child then calls `shm_get()` which maps the exact same physical page at the same virtual address in the child's page table.
- `deallocuvm` in `vm.c` was modified to avoid freeing `shm_page` when a process exits, keeping it available for other processes.

### Counting Semaphores
A lightweight counting semaphore system was implemented in the kernel (`sysproc.c`) with a static array of 10 semaphores (`struct sem`).
- `sem_init(id, val)`: Initializes the semaphore with the given value.
- `sem_down(id)`: Decrements the semaphore. If the count is 0, it sleeps using xv6's `sleep()` on the semaphore structure.
- `sem_up(id)`: Increments the semaphore and calls `wakeup()` to wake any sleeping processes.
These are protected by spinlocks to ensure atomicity.

## User Programs

### Question 1: Peterson's Algorithm (`peterson.c`)
- Purely uses user-space variables on the shared memory page.
- Processes busy-wait using `flag` and `turn` variables.
- Ensures a shared counter increments to 20 without lost updates.

### Question 2: Producer-Consumer (`prodcons.c`)
- A circular buffer of size 5 is stored in the shared memory page.
- Three semaphores are used: `empty` (init 5), `full` (init 0), and `mutex` (init 1).
- The producer waits on `empty` and signals `full`. The consumer waits on `full` and signals `empty`.

### Question 3: Readers-Writers (`readwrite.c`)
- Implements the first Readers-Writers problem (readers-preference).
- Uses a `rw_mutex` for writers and the first reader/last reader.
- A normal `mutex` protects the `read_count` variable.
- Spawns 3 readers and 2 writers, demonstrating concurrent reads and exclusive writes.

### Question 4: Dining Philosophers (`dining.c`)
- 5 philosophers represented by 5 processes.
- 5 semaphores represent forks (chopsticks).
- **Deadlock Avoidance Strategy:** Asymmetric approach. Odd-numbered philosophers pick up their left fork first, then their right fork. Even-numbered philosophers pick up their right fork first, then their left fork. This breaks the circular wait condition necessary for deadlock.

## Build and Run
1. Run `make qemu` to compile the xv6 kernel and user programs and boot it.
2. In the xv6 shell, run:
   - `peterson`
   - `prodcons`
   - `readwrite`
   - `dining`
