#include "types.h"
#include "user.h"

int main() {
    volatile int *shm = (volatile int*) shm_get();
    volatile int *flag = shm; // flag[0], flag[1]
    volatile int *turn = shm + 2;
    volatile int *shared_counter = shm + 3;

    flag[0] = 0;
    flag[1] = 0;
    *turn = 0;
    *shared_counter = 0;

    int pid = fork();
    if (pid < 0) {
        printf(1, "fork failed\n");
        exit();
    }

    int i = (pid == 0) ? 1 : 0; // i=0 for parent, i=1 for child
    int other = 1 - i;

    if (pid == 0) {
        // Child must also map the shared memory page
        shm = (volatile int*) shm_get();
        flag = shm;
        turn = shm + 2;
        shared_counter = shm + 3;
    }

    for(int iter = 0; iter < 10; iter++) {
        // Entry section
        flag[i] = 1;
        *turn = other;
        while(flag[other] == 1 && *turn == other) {
            // Busy wait
        }

        // Critical section
        (*shared_counter)++;
        printf(1, "Process %d in CS, counter = %d\n", i, *shared_counter);

        // Exit section
        flag[i] = 0;

        // Remainder section
        sleep(1);
    }
    
    if (pid > 0) {
        wait();
        printf(1, "Final counter value: %d\n", *shared_counter);
    }
    
    exit();
}
