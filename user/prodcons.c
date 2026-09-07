#include "types.h"
#include "user.h"

int main() {
    int *shm = (int*) shm_get();
    int *buffer = shm; // buffer of size 5
    int *in = shm + 5;
    int *out = shm + 6;

    *in = 0;
    *out = 0;

    int buf_size = 5;

    // semaphores: 0 = empty, 1 = full, 2 = mutex
    sem_init(0, buf_size); // empty slots
    sem_init(1, 0);        // full slots
    sem_init(2, 1);        // mutex

    int pid = fork();
    if (pid < 0) {
        printf(1, "fork failed\n");
        exit();
    }

    if (pid == 0) {
        // Consumer
        shm = (int*) shm_get();
        buffer = shm;
        in = shm + 5;
        out = shm + 6;

        for (int i = 0; i < 20; i++) {
            sem_down(1); // wait for full slot
            sem_down(2); // lock mutex

            int item = buffer[*out];
            printf(1, "Consumer removed item: %d from index %d\n", item, *out);
            *out = (*out + 1) % buf_size;

            sem_up(2);   // unlock mutex
            sem_up(0);   // signal empty slot

            sleep(10);   // simulate work
        }
        exit();
    } else {
        // Producer
        for (int i = 1; i <= 20; i++) {
            sem_down(0); // wait for empty slot
            sem_down(2); // lock mutex

            buffer[*in] = i;
            printf(1, "Producer inserted item: %d at index %d\n", i, *in);
            *in = (*in + 1) % buf_size;

            sem_up(2);   // unlock mutex
            sem_up(1);   // signal full slot

            sleep(5);    // simulate work
        }
        wait(); // wait for consumer
    }
    
    exit();
}
