#include "types.h"
#include "user.h"

int main() {
    int *shm = (int*) shm_get();
    int *shared_data = shm;
    int *read_count = shm + 1;

    *shared_data = 0;
    *read_count = 0;

    // semaphores: 3 = rw_mutex, 4 = mutex
    sem_init(3, 1); // rw_mutex
    sem_init(4, 1); // mutex

    for (int i = 0; i < 3; i++) {
        if (fork() == 0) {
            // Reader
            shm = (int*) shm_get();
            shared_data = shm;
            read_count = shm + 1;

            for(int j=0; j<2; j++) {
                sem_down(4); // mutex
                (*read_count)++;
                if (*read_count == 1) {
                    sem_down(3); // rw_mutex
                }
                sem_up(4); // mutex

                printf(1, "Reader %d reading data: %d, total readers: %d\n", getpid(), *shared_data, *read_count);
                sleep(10); // reading

                sem_down(4); // mutex
                (*read_count)--;
                if (*read_count == 0) {
                    sem_up(3); // rw_mutex
                }
                sem_up(4); // mutex

                sleep(5);
            }
            exit();
        }
    }

    for (int i = 0; i < 2; i++) {
        if (fork() == 0) {
            // Writer
            shm = (int*) shm_get();
            shared_data = shm;
            read_count = shm + 1;

            for(int j=0; j<2; j++) {
                sem_down(3); // rw_mutex

                (*shared_data)++;
                printf(1, "Writer %d wrote data: %d\n", getpid(), *shared_data);
                sleep(10); // writing

                sem_up(3); // rw_mutex

                sleep(20);
            }
            exit();
        }
    }

    // Wait for all children
    for (int i = 0; i < 5; i++) {
        wait();
    }
    
    exit();
}
