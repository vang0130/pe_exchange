#ifndef PE_PIPES_H
#define PE_PIPES_H

#include "pe_common.h"

extern struct order* create_buy_sell_order(char* str, int ind, 
                                           struct trader* curr_trader, 
                                           char (*valid_products)[MAX_PROD], 
                                           int num_products);

extern int check_for_match(struct order* new_order, 
                           struct orderbook** buy_orders, 
                           struct orderbook** sell_orders, 
                           struct trader** trader_list, 
                           struct printed_orderbook** printed_orders);

extern void add_order_prints(struct printed_orderbook** printed_orders, 
                             struct order* curr_order);

extern void add_order(struct order* curr_order, struct orderbook** buy_orders, 
                      struct orderbook** sell_orders);

extern struct order* find_order(int num_products, int trader_id, int id, 
                                struct orderbook** buy_orders, 
                                struct orderbook** sell_orders);

extern struct order* create_amended_order(int new_price, int new_quantity, 
                                          struct order* curr_orde);

extern struct orderbook* find_list(struct order* order_to_find, 
                                   struct orderbook** buy_orders, 
                                   struct orderbook** sell_orders);

extern void cancel_order_prints(struct order* curr_order, 
                                struct printed_orderbook** printed_orders);

extern void delete_order(struct order* curr_order, struct orderbook* list);

extern int cancel_order(struct order* curr_order, struct orderbook* list);

extern void print_bk_pos(int num_products, struct printed_orderbook** printed_orders, 
                         int num_traders, struct trader** trader_list);



void write_and_kill(int fd, char msg[MAX_LINE], pid_t pid);

void write_invalid(struct trader* curr_trader);

void write_accepted(struct order* curr_order, int trader_ind, 
                    struct trader** trader_list, int num_traders);

void write_amended(struct order* curr_order, int trader_ind, 
                   struct trader** trader_list, int num_traders);

void write_cancelled(int id, char product[MAX_PROD], int trader_ind, 
                     struct trader** trader_list, int num_traders, int is_buy);

void write_fill(struct order* buy_order, struct order* sell_order, int quantity, 
                struct trader* buy_trader, struct trader* sell_trader);

void write_market_open(struct trader** trader_list, int num_traders);



int process_msg(int fd, char* msg);

int buy_sell_msg(char msg[MAX_LINE], int trader_ind, 
                struct trader** trader_list, int num_traders, 
                struct printed_orderbook** printed_orders, 
                char (*valid_products)[MAX_PROD], int num_products, 
                struct orderbook** buy_orders, 
                struct orderbook** sell_orders);

struct order* check_valid_amend(int quantity, int price, int num_products, 
                                int trader_ind, 
                                struct orderbook** buy_orders, 
                                struct orderbook** sell_orders, 
                                char msg[MAX_LINE]);

int amend_msg(char msg[MAX_LINE], int trader_ind, struct trader** trader_list, 
              int num_traders, struct printed_orderbook** printed_orders,
              int num_products, struct orderbook** buy_orders, 
              struct orderbook** sell_orders);

int cancel_msg(char msg[MAX_LINE], int trader_ind, struct trader** trader_list, 
               int num_traders, struct printed_orderbook** printed_orders,
               int num_products, struct orderbook** buy_orders, 
               struct orderbook** sell_orders);

void parse_msg(char msg[MAX_LINE], int trader_ind, struct trader** trader_list, 
               int num_traders, struct printed_orderbook** printed_orders, 
               char (*valid_products)[MAX_PROD], int num_products, 
               struct orderbook** buy_orders, 
               struct orderbook** sell_orders);

#endif