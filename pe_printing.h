#ifndef PE_PRINTING_H
#define PE_PRINTING_H

#include "pe_common.h"


void print_products_starting(int num_products, char (*valid_products)[MAX_PROD]);

void print_trader_starting(struct trader* curr_trader, char* binary);

void print_orderbook(int num_products, 
                     struct printed_orderbook** printed_orders);

void print_positions(int num_traders, int num_products, 
                     struct trader** trader_list);

void print_bk_pos(int num_products, struct printed_orderbook** printed_orders, 
                  int num_traders, struct trader** trader_list);

void print_match(struct order* new_order, struct order* old_order, 
                 long long total_price, long long fee);

#endif