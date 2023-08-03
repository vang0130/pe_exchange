#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <ctype.h>
#include <time.h>


#define FIFO_EXCHANGE "/tmp/pe_exchange_%d"
#define FIFO_TRADER "/tmp/pe_trader_%d"
#define FEE_PERCENTAGE 1
#define MAX_LINE 50
#define QUEUE_LEN 100 // length of signal queues
#define MAX_PROD 17 // maximum length of a product name (incl \0)
#define LOG_PREFIX "[PEX]"

int trader_order_ID = 0; // keeps track of trader order ID
int sigusr_received = 0;

// writes and signals to the exchange
void write_and_kill(int fd, char* msg, pid_t exchange_pid) {
    write(fd, msg, strlen(msg));
    kill(exchange_pid, SIGUSR1);
}

// signal handler
void (handler)() {
    sigusr_received = 1;
}

// reads from exchange pipe
int process_msg(int fd, char* msg) {
    int reading = 1;
    int count = 0;
    while (reading == 1) {
        char c;
        int num = read(fd, &c, 1);
        if (num == 0) {
            return 0;
        }
        count++;
        if (c == ';') {
            msg[count - 1] = c;
            // add null biter
            msg[count] = '\0';
            reading = 0;
        }
        else {
            msg[count - 1] = c;
        }
    }
    return 0;
}


int main(int argc, char ** argv) {
    // wrong number of command line arguments
    if (argc != 2) {
        return 1;
    }
    struct sigaction sa = {0};
    sa.sa_handler = &handler;
    sigaction(SIGUSR1, &sa, NULL);
 
    int trader_id = atoi(argv[1]);

    // connect to named pipes
    char* ex_pipe_name = calloc(strlen(FIFO_EXCHANGE) + strlen(argv[1]) + 1, 1);
    char* tr_pipe_name = calloc(strlen(FIFO_TRADER) + strlen(argv[1]) + 1, 1);
    // create pipe name strings
    sprintf(ex_pipe_name, FIFO_EXCHANGE, trader_id);
    sprintf(tr_pipe_name, FIFO_TRADER, trader_id);
    // connect to the exchange pipes
    // store the fds of the pipes
    int ex_pipe_fd;
    int tr_pipe_fd;
    if ((ex_pipe_fd = open(ex_pipe_name, O_RDONLY)) < 0) {
        perror("can't open exchange pipe");
        return 1;
    }
    if ((tr_pipe_fd = open(tr_pipe_name, O_WRONLY)) < 0) {
        perror("can't open trader pipe");
        return 1;
    }
    pid_t exchange_pid = getppid(); // save pid of the exchange
    char msg[MAX_LINE];
    // wait for market open
    FILE* fp = fopen("tests/E2E/match_orders/trader1.in", "r");
    sleep(10);
    while (fgets(msg, MAX_LINE, fp) != NULL) {
        msg[strlen(msg) - 1] = '\0'; // remove newline
        write_and_kill(tr_pipe_fd, msg, exchange_pid);
        memset(msg, 0, MAX_LINE);
        sleep(5);
        if (sigusr_received == 1) {
            sigusr_received = 0;
            process_msg(ex_pipe_fd, msg);
            memset(msg, 0, MAX_LINE);
        }
        else {
            break;
        }
    }
    unlink(ex_pipe_name);
    unlink(tr_pipe_name);
    close(ex_pipe_fd);
    close(tr_pipe_fd);
    free(ex_pipe_name);
    free(tr_pipe_name);
    return 0;
}