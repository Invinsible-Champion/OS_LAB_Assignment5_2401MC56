#include "types.h"
#include "user.h"

// States for printing
char *states[] = {"THINKING", "HUNGRY", "EATING"};

void philosopher(int id) {
    int left = id;
    int right = (id + 1) % 5;
    
    // Semaphores 0 to 4 are forks
    // We use asymmetric strategy:
    // Odd philosophers pick up left fork first, then right.
    // Even philosophers pick up right fork first, then left.

    int first = (id % 2 != 0) ? left : right;
    int second = (id % 2 != 0) ? right : left;

    for (int i = 0; i < 5; i++) {
        printf(1, "Philosopher %d: THINKING -> HUNGRY\n", id);
        
        sem_down(first);
        sem_down(second);

        printf(1, "Philosopher %d: HUNGRY -> EATING\n", id);
        sleep(10); // eating

        sem_up(first);
        sem_up(second);
        
        printf(1, "Philosopher %d: EATING -> THINKING\n", id);
        sleep(10); // thinking
    }
}

int main() {
    // initialize 5 semaphores for 5 forks
    for (int i = 0; i < 5; i++) {
        sem_init(i, 1);
    }

    for (int i = 0; i < 5; i++) {
        int pid = fork();
        if (pid < 0) {
            printf(1, "fork failed\n");
            exit();
        }
        if (pid == 0) {
            philosopher(i);
            exit();
        }
    }

    for (int i = 0; i < 5; i++) {
        wait();
    }
    
    exit();
}
