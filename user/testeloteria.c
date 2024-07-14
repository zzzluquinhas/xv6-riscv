#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/pstat.h"
#include "user/user.h"

#define USE_YIELD
#define MAX_CHILDREN 32
#define LARGE_TICKET_COUNT 100000
#define MAX_YIELDS_FOR_SETUP 100

__attribute__((noreturn))
void yield_forever() {
    while (1) {
        yield();
    }
}

__attribute__((noreturn))
void run_forever() {
    while (1) {
        __asm__("");
    }
}

int spawn(int tickets) {
    int pid = fork();
    if (pid == 0) {
        settickets(tickets);
        yield();
#ifdef USE_YIELD
        yield_forever();
#else
        run_forever();
#endif
    } else if (pid != -1) {
        return pid;
    } else {
        fprintf(2, "error in fork\n");
        return -1;
    }
}

int find_index_of_pid(int *list, int list_size, int pid) {
    for (int i = 0; i < list_size; ++i) {
        if (list[i] == pid)
            return i;
    }
    return -1;
}

void wait_for_ticket_counts(int num_children, int *pids, int *tickets) {
    for (int yield_count = 0; yield_count < MAX_YIELDS_FOR_SETUP; ++yield_count) {
        yield();
        int done = 1;
        struct pstat info;
        getpinfo(&info);
        for (int i = 0; i < num_children; ++i) {
            int index = find_index_of_pid(info.pid, NPROC, pids[i]);
            if (info.tickets[index] != tickets[i]) done = 0;
        }
        if (done)
            break;
    }
}

int main(int argc, char *argv[])
{
    if (argc != 3) {
        fprintf(2, "usage: %s seconds tickets\n"
                  "       seconds is the number of time units to run for\n"
                  "       tickets is the number of tickets to give to subprocesses in ratio 1:2:3\n",
                  argv[0]);
        exit(0);
    }
    int tickets_for[MAX_CHILDREN];
    int active_pids[MAX_CHILDREN];
    int num_seconds = atoi(argv[1]);
    int num_children = 3;
    /* give us a lot of ticket so we don't get starved */
    settickets(LARGE_TICKET_COUNT);
    for (int i = 0; i < num_children; ++i) {
        int tickets = atoi(argv[2]) * (i+1);
        tickets_for[i] = tickets;
        active_pids[i] = spawn(tickets);
    }
    wait_for_ticket_counts(num_children, active_pids, tickets_for);
    struct pstat before, after;
    for(int i = 0; i < NPROC; i++) {
		before.inuse[i] = 0;
		after.inuse[i] = 0;
	}
    getpinfo(&before);
    sleep(num_seconds);
    getpinfo(&after);
    for (int i = 0; i < num_children; ++i) {
        kill(active_pids[i]);
    }
    for (int i = 0; i < num_children; ++i) {
        wait(0);
    }
	int numBefore = 0, numAfter = 0;
	for (int i = 0; i < NPROC; i++) {
		if (before.inuse[i]) numBefore++;
		if (after.inuse[i]) numAfter++;
	}
    if (numBefore == 0 || numAfter == 0) {
        fprintf(2, "getpinfo doesn't have active processes -- not changed by syscall?\n");
        exit(1);
    }
    for (int i = 0; i < num_children; ++i) {
        int before_index = find_index_of_pid(before.pid, NPROC, active_pids[i]);
        int after_index = find_index_of_pid(after.pid, NPROC, active_pids[i]);
        if (before_index == -1)
            fprintf(2, "child %d did not exist for getpinfo before parent slept\n", i);
        if (after_index == -1)
            fprintf(2, "child %d did not exist for getpinfo after parent slept\n", i);
        if (before_index == -1 || after_index == -1) {
            fprintf(1, "%d\t--unknown--\n", tickets_for[i]);
        } else {
            if (before.tickets[before_index] != tickets_for[i]) {
                fprintf(2, "child %d had wrong number of tickets in getprocessinfo before parent slept\n", i);
            }
            if (after.tickets[after_index] != tickets_for[i]) {
                fprintf(2, "child %d had wrong number of tickets in getprocessinfo after parent slept\n", i);
            }
            fprintf(1, "%d tickets: %d ticks\n", tickets_for[i], after.ticks[after_index] - before.ticks[before_index]);
        }
    }
    exit(0);
}
