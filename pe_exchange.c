/**
 * comp20MAX_PROD - assignment 3
 * <vanessa wang>
 * <vwan0130>
 */

#include "pe_exchange.h"

/////////////////////////////// GLOBAL VARIABLES ///////////////////////////////

// queues for sigusr1 and sigchld
pid_t sigusr_queue[QUEUE_LEN]; // circular queue
int sigusr_queue_len = 0; // length of queue
int sigusr_front = 0; // current front index of queue
int sigusr_back = 0; // current back index of queue

pid_t sigchld_queue[QUEUE_LEN];
int sigchld_queue_len = 0;
int sigchld_front = 0;
int sigchld_back = 0;

/////////////////////////////// SIGNAL FUNCTIONS ///////////////////////////////

// handler for SIGUSR1
void sigusr_handler(int signum, siginfo_t* siginfo, void* options) {
    sigusr_queue[sigusr_back] = siginfo->si_pid; // add pid to array
    sigusr_back++; // back of array increases
    if (sigusr_back == 50) {
        sigusr_back = 0; // circle back
    }
    sigusr_queue_len++; // increase length of queue
}

// handler for SIGCHLD
void sigchld_handler(int signum, siginfo_t* siginfo, void* options) {
    sigchld_queue[sigchld_back] = siginfo->si_pid;
    sigchld_back++;
    if (sigchld_back == 50) {
        sigchld_back = 0;
    }
    sigchld_queue_len++;
    waitpid(siginfo->si_pid, NULL, 0);
}

// remove something from signal queue
void pop_sigusr() {
    sigusr_queue[sigusr_front] = 0; // front is removed (front is oldest signal)
    sigusr_front++; // front is now one to the right
    if (sigusr_front == 50) {
        sigusr_front = 0; // circle back
    }
    sigusr_queue_len--; // decrement length of queue
}

// remove something from SIGCHLD queue
void pop_sigchld() {
    sigchld_queue[sigchld_front] = 0;
    sigchld_front++;
    if (sigchld_front == 50) {
        sigchld_front = 0;
    }
    sigchld_queue_len--;
}

// set the sigusr1 and sigchld handlers
void set_handlers(struct sigaction* sigusr, struct sigaction* sigchld) {
    sigusr->sa_sigaction = sigusr_handler;
    sigusr->sa_flags = SA_SIGINFO;
    sigaction(SIGUSR1, sigusr, NULL);

    sigchld->sa_sigaction = sigchld_handler;
    sigchld->sa_flags = SA_SIGINFO;
    sigaction(SIGCHLD, sigchld, NULL);
}


int main(int argc, char** argv) {
    if (argc < 3) {
        printf("Too few arguments\n");
        return 1;
    }

    // signal handler!
    struct sigaction sigusr = {0};
    struct sigaction sigchld = {0};
    set_handlers(&sigusr, &sigchld);

    int num_products = get_num_products(argv[1]);
    if (num_products == -1) {
        return 1;
    }
    char (*valid_products)[MAX_PROD] = calloc(num_products, sizeof(char[MAX_PROD]));
    int successful_read = get_products(argv[1], num_products, valid_products);
    if (successful_read != 0) {
        free(valid_products);
        return 1;
    }

    print_products_starting(num_products, valid_products);

    // fork all child processes, save info
    int num_traders = argc - 2;
    pid_t* pids = calloc(num_traders, sizeof(pid_t));

    // all trader info
    struct trader** trader_list = calloc(num_traders, sizeof(struct trader*));
    int successful_fork = fork_and_pipes(num_traders, argv, 
                                         trader_list, num_products, pids);
    if (successful_fork != -1) {
        free_early(trader_list, num_products, 
                   valid_products, successful_fork - 1);
        return 1;
    }

    int poll_rd = epoll_create(num_traders);
    if (poll_rd < 0) {
        perror("Can't create epoll: ");
        free_early(trader_list, num_products, valid_products, num_traders);
        return 1;
    }
    // create all epolls to watch
    struct epoll_event* all_events = calloc(num_traders, 
                                     sizeof(struct epoll_event));
    struct printed_orderbook** printed_orders = calloc(num_products, 
                                              sizeof(struct printed_orderbook));
    struct orderbook** buy_orders = calloc(num_products, 
                                    sizeof(struct orderbook*));
    struct orderbook** sell_orders = calloc(num_products, 
                                     sizeof(struct orderbook*));
    initialise_product_list(num_products, valid_products, printed_orders);
    initialise_prod_orders(buy_orders, sell_orders, num_products);
    initialise_trader_inventory(num_traders, num_products, 
                                valid_products, trader_list);
    
    // send MARKET OPEN to all traders, and signal
    write_market_open(trader_list, num_traders);

    // create epoll
    create_epoll(num_traders, trader_list, &poll_rd);

    char msg[MAX_LINE] = {0}; 
    char printing_msg[MAX_LINE] = {0};
    int num_active = num_traders;
    int eintr_epoll = 0;
    while (1) {
        // if epoll hasn't been interrupted by a signal, pause and wait for sig
        if (eintr_epoll == 0) {
            pause();
        }
        eintr_epoll = 0;
        // check if any sigusr1 has been received
        while (sigusr_queue_len > 0) {
            
            int trader_ind = -1;
            // save pid of trader
            pid_t pid = sigusr_queue[sigusr_front];
            pop_sigusr(); // delete from queue
            for (int i = 0; i < num_traders; i++) {
                if (pid == pids[i]) {
                    trader_ind = i; // find relevant trader
                    break;
                }
            }
            // read and print message
            int read = process_msg(trader_list[trader_ind]->tr_fd, msg);
            if (read == 1) {
                free_all(trader_list, num_products, printed_orders, num_traders, 
                         buy_orders, sell_orders, valid_products, all_events);
                // return 1;
            }
            if (read == 0) {
                free_all(trader_list, num_products, printed_orders, num_traders, 
                         buy_orders, sell_orders, valid_products, all_events);
                // return 1;
            }
            strcpy(printing_msg, msg);
            printing_msg[strlen(printing_msg) - 1] = '\0'; // cut off semicolon
            printf("%s [T%d] Parsing command: <%s>\n", 
                    LOG_PREFIX, trader_ind, printing_msg);
            // deal with message
            parse_msg(msg, trader_ind, trader_list, num_traders, printed_orders, 
                      valid_products, num_products, buy_orders, sell_orders);
            memset(msg, 0, MAX_LINE);
            memset(printing_msg, 0, MAX_LINE);
        }
        // check if any sigchld has been received
        while (sigchld_queue_len > 0 && sigusr_queue_len == 0) {
            int pid = sigchld_queue[sigchld_front];
            pop_sigchld();
            // get pid of child
            for (int i = 0; i < num_traders; i++) {
                if (pid == pids[i]) {
                    if (trader_list[i]->alive != 0) {
                        num_active--; // decrement num active traders
                        // kill trader
                        trader_died(poll_rd, i, trader_list[i], &all_events[i]);
                    }
                }
            }

            // if no active traders, end program
            if (num_active == 0) {
                all_traders_died(trader_list, num_products, printed_orders, 
                                 num_traders, buy_orders, sell_orders, 
                                 valid_products, all_events);
            }
        }

        // check if any traders have disconnected pipes
        // this is because a trader can disconnect pipes without sending sigchld
        int num_returned = epoll_wait(poll_rd, all_events, 1, 1000);
        if (num_returned <= 0) {
            // if not EINTR, exit
            if (errno != EINTR) {
                perror("Epoll_wait failed");
                free_all(trader_list, num_products, printed_orders, num_traders, 
                         buy_orders, sell_orders, valid_products, all_events);
                return 1;
            }
            // if signal interrupt, go back to start of while loop
            // to deal with sigusr or sigchld
            else {
                eintr_epoll = 1;
                continue;
            }
        }
        else {
            // if trader closed pipes, kill trader
            for (int i = 0; i < num_returned; i++) {
                int trader_ind = all_events[i].data.u32;
                if (trader_list[trader_ind]->alive != 0) {
                    trader_died(poll_rd, trader_ind, 
                                trader_list[trader_ind], &all_events[i]);
                    num_active--;
                    if (num_active == 0) {
                        all_traders_died(trader_list, num_products, 
                                         printed_orders, num_traders, 
                                         buy_orders, sell_orders, 
                                         valid_products, all_events);
                    }
                }
            }
        }
    }
}