#include "include.h"
#include <signal.h>
#include <unistd.h>

#define SHM_NAME "/shared_memory"
#define SEM_NAME "/sempahore"

pid_t process_pids[6];
volatile sig_atomic_t keep_running = 1;

void handle_sigusr2(int sig) {
    printf("Received SIGUSR2 from one of the target processes\n");
}

void terminate_all_processes(int j) {
    printf("\nTERMINATING ALL PROCESSES\n");
    if(keep_running){
        for (int i = 0; i < 6; i++) {
            if(i != j){
                if (kill(process_pids[i], SIGTERM) == -1) {
                    perror("kill for termination");
                }
            }
        }
    }       
    printf("All processes have been closed...\n");
    keep_running = 0;
}

void check_process() {
    for (int i = 0; i < 6; i++) 
    {
        if (keep_running)
        {
            if (kill(process_pids[i], SIGUSR1) == -1) {
                if (errno == ESRCH) {
                    printf("Process %d does not exist or is not running.\n", process_pids[i]);
                    terminate_all_processes(i);
                    
                } else {
                    perror("kill");
                }
            } else {
                printf("Sent SIGUSR1 to process %d\n", process_pids[i]);
            }
        }
    }
    

    

}

void handle_sigint_or_sigterm(int sig) {
    if (sig == SIGINT) {
        printf("\nReceived SIGINT, terminating...\n");
    } else if (sig == SIGTERM) {
        printf("\nReceived SIGTERM, cleaning up and terminating...\n");
    }
    terminate_all_processes(7);
}

int main() {
    int shm_fd = shm_open(SHM_NAME, O_RDWR, 0666);
    if (shm_fd == -1) {
        perror("shm_open");
        exit(1);
    }

    PidData *data = mmap(0, sizeof(PidData), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (data == MAP_FAILED) {
        perror("mmap");
        exit(1);
    }

    sem_t *sem = sem_open(SEM_NAME, 0);
    if (sem == SEM_FAILED) {
        perror("sem_open");
        exit(1);
    }

    sem_wait(sem);
    printf("Semaphore blocked\n");
    data->watchdog_pid = getpid();
    printf("WD PID scritto nella memoria condivisa: %d\n", data->watchdog_pid);
    sem_post(sem);

    bool flag = true;
    printf("Checking Processes PIDS\n\n");
    while (flag) {
        sem_wait(sem);
        process_pids[0] = data->server_pid;
        process_pids[1] = data->drone_pid;
        process_pids[2] = data->display_pid;
        process_pids[3] = data->map_pid;
        process_pids[4] = data->obstacle_pid;
        process_pids[5] = data->target_pid;
        if (process_pids[0] == 0 || process_pids[1] == 0 || process_pids[2] == 0 || process_pids[3] == 0 || process_pids[4] == 0 || process_pids[5] == 0) {
            printf("Can't reach at least one of the processes...\n\n");
        } else {
            flag = false;
            printf("PID fetch completed!\n");
        }
        sem_post(sem);
        sleep(2);
    }

    printf("Server PID: %d\n", process_pids[0]);
    printf("Drone PID: %d\n", process_pids[1]);
    printf("Target PID: %d\n", process_pids[5]);
    printf("Obstacle PID: %d\n", process_pids[4]);
    printf("Map PID: %d\n", process_pids[3]);
    printf("Dispaly PID: %d\n", process_pids[2]);

    struct sigaction sa, sa_int;
    sa.sa_handler = handle_sigusr2;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGUSR2, &sa, NULL);

    sa_int.sa_handler = handle_sigint_or_sigterm;
    sigemptyset(&sa_int.sa_mask);
    sa_int.sa_flags = 0;
    sigaction(SIGINT, &sa_int, NULL);
    sigaction(SIGTERM, &sa_int, NULL);

    while (keep_running) {
        sleep(5);
        printf("Watchdog in esecuzione...\n");
        check_process();
        fflush(stdout);
        sleep(5);
    }

    // Cleanup code
    munmap(data, sizeof(PidData));
    close(shm_fd);
    sem_close(sem);
    return 0;
}
