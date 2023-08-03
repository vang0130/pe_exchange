#ifndef PE_COMMON_H
#define PE_COMMON_H
#define _GNU_SOURCE


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/epoll.h>
#include <sys/wait.h>
#include <errno.h>
#include <ctype.h>
#include <math.h>
#include <time.h>

#define FIFO_EXCHANGE "/tmp/pe_exchange_%d"
#define FIFO_TRADER "/tmp/pe_trader_%d"
#define FEE_PERCENTAGE 1
#define MAX_LINE 50
#define QUEUE_LEN 100 // length of signal queues
#define MAX_PROD 17 // maximum length of a product name (incl \0)
#define LOG_PREFIX "[PEX]"


// GLOBALS
extern long long exchange_fees;

extern pid_t sigusr_queue[QUEUE_LEN]; 
extern int sigusr_queue_len; 
extern int sigusr_front; 
extern int sigusr_back; 

extern pid_t sigchld_queue[QUEUE_LEN];
extern int sigchld_queue_len;
extern int sigchld_front;
extern int sigchld_back;

// STRUCT DECLARATIONS
typedef struct order order;
typedef struct orderbook orderbook;
typedef struct printed_order printed_order;
typedef struct printed_orderbook printed_orderbook;
typedef struct trader trader;
typedef struct trader_balances trader_balances;


struct order { // each order within buy and sell books
    char product[MAX_PROD];
    int quantity;
    int price;
    int order_ID;
    int product_ID;
    int trader_ID;
    int is_buy;
    struct order* prev;
    struct order* next;
};

struct orderbook { // buy and sell book for each product
    struct order* head;
    struct order* tail;
};

struct printed_order { // each order within printed_order list
    int quantity;
    int price;
    int num_orders;
    struct printed_order* next;
};

struct printed_orderbook { // for each product
	char product[MAX_PROD];
	struct printed_order* head_buy;
	struct printed_order* head_sell;
	int buy_len;
	int sell_len;
};

struct trader { // for each trader
    int ID;
    pid_t pid;
    int order_ID;
    char ex_name[MAX_LINE];
    char tr_name[MAX_LINE];
    int ex_fd;
    int tr_fd;
    struct trader_balances** trader_prod_info; // not implemented yet
    int alive;
};

struct trader_balances { // for each product of each trader
    char product[MAX_PROD];
    int quantity;
    long long balance;
};

#endif