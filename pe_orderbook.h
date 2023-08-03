#ifndef PE_ORDERBOOK_H
#define PE_ORDERBOOK_H

#include "pe_common.h"


extern void write_fill(struct order* buy_order, struct order* sell_order, 
                       int quantity, struct trader* buy_trader, 
                       struct trader* sell_trader);

extern long long round_int(long double n);

extern void print_match(struct order* new_order, struct order* old_order, 
                        long long total_price, long long fee);


struct order* create_order(char* str, int ind, struct trader* curr_trader, 
                           char (*valid_products)[MAX_PROD], int num_products,
                           int is_buy);

struct order* create_buy_sell_order(char* str, int ind, 
                                    struct trader* curr_trader, 
                                    char (*valid_products)[MAX_PROD], 
                                    int num_products);

struct printed_order* create_printed_order(struct order* curr_order);

struct order* create_amended_order(int new_price, int new_quantity, 
                                   struct order* curr_orde);

void set_len(struct printed_orderbook* curr_prod, int is_buy);



void add_to_prints(struct printed_orderbook* curr_prod, 
                   struct order* curr_order, int is_buy);

void add_order_prints(struct printed_orderbook** printed_orders, 
                      struct order* curr_order);

void add_to_tail(struct order* curr_order, struct orderbook* list);

void add_order(struct order* curr_order, struct orderbook** buy_orders, 
               struct orderbook** sell_orders);



void remove_from_products(struct printed_order* node, 
                          struct printed_orderbook* curr_prod, int is_buy);

void delete_order_prints(struct printed_orderbook* curr_prod, 
                         struct order* curr_order);

void cancel_order_prints(struct order* curr_order, 
                         struct printed_orderbook** printed_orders);

void delete_order(struct order* curr_order, struct orderbook* list);

int cancel_order(struct order* curr_order, struct orderbook* list);


struct order* find_order(int num_products, int trader_id, int id, 
                         struct orderbook** buy_orders, 
                         struct orderbook** sell_orders);

struct orderbook* find_list(struct order* order_to_find, 
                            struct orderbook** buy_orders, 
                            struct orderbook** sell_orders);


void update_inventory(struct trader* buy_trader, struct trader* sell_trader, 
                      long long total_price, int quantity, int prod_ind);

struct order* find_best_sell(struct order* buy_order, 
                             struct orderbook** sell_orders);

struct order* find_best_buy(struct order* sell_order, 
                            struct orderbook** buy_orders);

int match_new_buy(struct trader** trader_list, struct order* buy_order, 
                  struct order* sell_order, 
                  struct printed_orderbook** printed_orders, 
                  struct orderbook** buy_orders, 
                  struct orderbook** sell_orders);

int match_new_sell(struct trader** trader_list, struct order* sell_order, 
                   struct order* buy_order, 
                   struct printed_orderbook** printed_orders, 
                   struct orderbook** buy_orders, 
                   struct orderbook** sell_orders);

int check_for_match(struct order* new_order, struct orderbook** buy_orders, 
                    struct orderbook** sell_orders, 
                    struct trader** trader_list, 
                    struct printed_orderbook** printed_orders);

#endif