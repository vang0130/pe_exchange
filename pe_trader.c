#include "pe_trader.h"

int signal_received = 0; // flag for sigusr
int trader_order_ID = 0; // keeps track of trader order ID

// signal handler
void (handler)() {
    signal_received = 1;
}

// reads from exchange pipe
int process_msg(int fd, char* msg) {
    int reading = 1;
    int count = 0;
    while (reading == 1) {
        char c;
        int num = read(fd, &c, 1);
        if (num == -1) {
            perror("Reading error: ");
            return 1;
        }
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

// counts strlen of a number
int strlen_digit(int x) {
    int count = 0;
    do {
        x /= 10;
        ++count;
    } while (x != 0);
    return count;
}

// finds length of the buy string to be written
int find_buy_str_len(struct order* curr_order) {
    int len = 4; // "BUY "
    len += strlen_digit(curr_order->order_ID) + 1; // "<Next Order ID> "
    len += strlen(curr_order->product) + 1; // "<product> "
    len += strlen_digit(curr_order->quantity) + 1; // "<quantity> "
    len += strlen_digit(curr_order->price) + 1; // "<price> "
    len += 1; // ";" + '\0'
    return len;
}

// creates a buy order from a sell string
int create_buy_order(char* sell_str, struct order* curr_buy) {
    char market[MAX_LINE];
    char sell[MAX_LINE];
    sscanf(sell_str, "%s %s %s %d %d;", market, sell, curr_buy->product, 
           &curr_buy->quantity, &curr_buy->price);
    // only create buy order if msg is MARKET SELL
    if (strcmp(market, "MARKET") == 0) {
        if (strcmp(sell, "SELL") == 0) {
            curr_buy->order_ID = trader_order_ID;
            trader_order_ID++;
            return 1;
        }
        else {
            return 0;
        }
    }
    else {
        return 0;
    }
}

// creates a buy string from a buy order
// buy string will be written
char* create_buy_str(struct order* curr_order) {
    int len = find_buy_str_len(curr_order);
    char* buy_str = calloc(len, 1);
    sprintf(buy_str, "BUY %d %s %d %d;", curr_order->order_ID, 
            curr_order->product, curr_order->quantity, curr_order->price);
    return buy_str;
}

// writes and signals to the exchange
void write_and_kill(int fd, char* msg, pid_t exchange_pid) {
    write(fd, msg, strlen(msg));
    kill(exchange_pid, SIGUSR1);
}


int main(int argc, char ** argv) {
    // set up the signal handler
    struct sigaction sa = {0};
    sa.sa_handler = &handler;
    sigaction(SIGUSR1, &sa, NULL);

    // wrong number of command line arguments
    if (argc != 2) {
        return 1;
    }
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

    int running = 1;
    pid_t exchange_pid = getppid(); // save pid of the exchange
    char msg[MAX_LINE];
    struct timespec time = {.tv_sec = 0, .tv_nsec = 3e8};   

    while (running == 1) {
        pause();
        // if sigusr1 received from exchange
        if (signal_received == 1) {
            signal_received = 0;
            // read the message from exchange pipe
            process_msg(ex_pipe_fd, msg);

            struct order curr_buy = {0};
            int create_buy = create_buy_order(msg, &curr_buy);
            if (create_buy == 0) {
                continue;
            }
            // if quantity too large
            if (curr_buy.quantity >= 1000) {
                running = 0;
                break;
            }
            // create buy order str and wripe to trader pipe
            char* buy_str = create_buy_str(&curr_buy);
            write_and_kill(tr_pipe_fd, buy_str, exchange_pid);
            
            // after we have written to the exchange, wait for accept message
            int wait = 1;
            while (wait == 1) {
                // wait for 0.3 seconds
                nanosleep(&time, NULL);
                if (signal_received == 1) {
                    signal_received = 0;
                    process_msg(ex_pipe_fd, msg);
                    wait = 0;
                    break;
                }
                // if signal not received within 0.3 seconds, resend
                else {
                    kill(exchange_pid, SIGUSR1);
                }
            }
            free(buy_str);
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