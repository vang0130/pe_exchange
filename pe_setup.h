#ifndef PE_SETUP_H
#define PE_SETUP_H

#include "pe_common.h"

extern void print_trader_starting(struct trader* curr_trader, char* binary);


int is_alphanumeric(char* line);

int is_num(char* str);

long long round_int(long double n);


void sigusr_handler(int signum, siginfo_t* siginfo, void* options);

void sigchld_handler(int signum, siginfo_t* siginfo, void* options);

void pop_sigusr();

void pop_sigchld();

void set_handlers(struct sigaction* sigusr, struct sigaction* sigchld);


void free_orders(struct orderbook** buy_orders, struct orderbook** sell_orders, 
                 int num_products);

void free_printed_orders(struct printed_order* head);

void free_all(struct trader** trader_list, int num_products, 
              struct printed_orderbook** printed_orders, int num_traders, 
              struct orderbook** buy_orders, 
              struct orderbook** sell_orders, char (*valid_products)[MAX_PROD],
              struct epoll_event* all_events);

void free_early(struct trader** trader_list, int num_products, 
                char (*valid_products)[MAX_PROD], int num_traders);



void trader_died(int poll_rd, int i, struct trader* curr_trader, 
                 struct epoll_event* curr_event);

void all_traders_died(struct trader** trader_list, int num_products, 
                      struct printed_orderbook** printed_orders, 
                      int num_traders, struct orderbook** buy_orders, 
                      struct orderbook** sell_orders, 
                      char (*valid_products)[MAX_PROD], 
                      struct epoll_event* all_events);


void initialise_product_list(int num_products, char (*valid_products)[MAX_PROD], 
                             struct printed_orderbook** printed_orders);

void initialise_trader(struct trader* curr_trader, int i, int fds[2], 
                       char buffer_ex[MAX_LINE], char buffer_tr[MAX_LINE], 
                       int num_products, int fork_res);

void initialise_prod_orders(struct orderbook** buy_orders, 
                            struct orderbook** sell_orders, int num_products);

void initialise_trader_inventory(int num_traders, int num_products, 
                                 char (*valid_products)[MAX_PROD], 
                                 struct trader** trader_list);


int fork_and_pipes(int num_traders, char** argv, 
                   struct trader** trader_list, int num_products, pid_t* pids);

void create_epoll(int num_traders, struct trader** trader_list, int* poll_rd);

int get_num_products(char* filename);

int get_products(char* filename, int num_products, 
                 char (*valid_products)[MAX_PROD]);

#endif