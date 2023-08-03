#ifndef PE_EXCHANGE_H
#define PE_EXCHANGE_H

#include "pe_common.h"

extern int get_num_products(char* filename);

extern int get_products(char* filename, int num_products, 
                        char (*valid_products)[MAX_PROD]);

extern void print_products_starting(int num_products, 
                                    char (*valid_products)[MAX_PROD]);

extern void print_trader_starting(struct trader* curr_trader, char* binary);

extern int fork_and_pipes(int num_traders, char** argv, 
                          struct trader** trader_list, int num_products, 
                          pid_t* pids);

extern void initialise_product_list(int num_products, char (*valid_products)[MAX_PROD], 
                             struct printed_orderbook** printed_orders);

extern void initialise_trader(struct trader* curr_trader, int i, int fds[2], 
                       char buffer_ex[MAX_LINE], char buffer_tr[MAX_LINE], 
                       int num_products, int fork_res);

extern void initialise_prod_orders(struct orderbook** buy_orders, 
                            struct orderbook** sell_orders, int num_products);

extern void initialise_trader_inventory(int num_traders, int num_products, 
                                 char (*valid_products)[MAX_PROD], 
                                 struct trader** trader_list);

extern void create_epoll(int num_traders, struct trader** trader_list, int* poll_rd);

extern int process_msg(int fd, char* msg);

extern void parse_msg(char msg[MAX_LINE], int trader_ind, 
                      struct trader** trader_list, 
                      int num_traders, 
                      struct printed_orderbook** printed_orders, 
                      char (*valid_products)[MAX_PROD], int num_products, 
                      struct orderbook** buy_orders, 
                      struct orderbook** sell_orders);

extern void trader_died(int poll_rd, int i, struct trader* curr_trader, 
                 struct epoll_event* curr_event);

extern void all_traders_died(struct trader** trader_list, int num_products, 
                      struct printed_orderbook** printed_orders, 
                      int num_traders, struct orderbook** buy_orders, 
                      struct orderbook** sell_orders, 
                      char (*valid_products)[MAX_PROD], 
                      struct epoll_event* all_events);

extern void free_early(struct trader** trader_list, int num_products, 
                char (*valid_products)[MAX_PROD], int num_traders);

extern void write_market_open(struct trader** trader_list, int num_traders);

extern void free_all(struct trader** trader_list, int num_products, 
              struct printed_orderbook** printed_orders, int num_traders, 
              struct orderbook** buy_orders, 
              struct orderbook** sell_orders, char (*valid_products)[MAX_PROD],
              struct epoll_event* all_events);



void sigusr_handler(int signum, siginfo_t* siginfo, void* options);

void sigchld_handler(int signum, siginfo_t* siginfo, void* options);

void pop_sigusr();

void pop_sigchld();

void set_handlers(struct sigaction* sigusr, struct sigaction* sigchld);


#endif