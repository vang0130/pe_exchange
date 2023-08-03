#include "pe_printing.h"


///////////////////////////// PRINTING FUNCTIONS //////////////////////////////

// prints all products at beginning
void print_products_starting(int num_products, char (*valid_products)[MAX_PROD]) {
    printf("%s Starting\n", LOG_PREFIX);
    printf("%s Trading %d products: ", LOG_PREFIX, num_products);
    for (int i = 0; i < num_products; i++) {
        printf("%s", valid_products[i]);
        if (i != num_products - 1) {
            printf(" ");
        }
    }
    printf("\n");
}

// print trader starting
void print_trader_starting(struct trader* curr_trader, char* binary) {
    printf("%s Created FIFO %s\n", LOG_PREFIX, curr_trader->ex_name);
    printf("%s Created FIFO %s\n", LOG_PREFIX, curr_trader->tr_name);
    printf("%s Starting trader %d (%s)\n", LOG_PREFIX, curr_trader->ID, binary);
    printf("%s Connected to %s\n", LOG_PREFIX, curr_trader->ex_name);
    printf("%s Connected to %s\n", LOG_PREFIX, curr_trader->tr_name);
}

// prints orderbook
void print_orderbook(int num_products, 
                     struct printed_orderbook** printed_orders) {
    printf("%s	--ORDERBOOK--\n", LOG_PREFIX);
    for (int i = 0; i < num_products; i++) {
        printf("%s\tProduct: %s; Buy levels: %d; Sell levels: %d\n", 
                LOG_PREFIX, printed_orders[i]->product, 
                printed_orders[i]->buy_len, printed_orders[i]->sell_len);
        // print sell orders first
        struct printed_order* temp = printed_orders[i]->head_sell;
        while (temp != NULL) {
            if (temp->num_orders > 1) {
                printf("%s\t\tSELL %d @ $%d (%d orders)\n", LOG_PREFIX, 
                        temp->quantity, temp->price, temp->num_orders);
            }
            else if (temp->num_orders == 1) {
                printf("%s\t\tSELL %d @ $%d (%d order)\n", LOG_PREFIX, 
                        temp->quantity, temp->price, temp->num_orders);
            }
            temp = temp->next;
        }
        // print buy orders
        temp = printed_orders[i]->head_buy;
        while (temp != NULL) {
            if (temp->num_orders > 1) {
                printf("%s\t\tBUY %d @ $%d (%d orders)\n", LOG_PREFIX, 
                        temp->quantity, temp->price, temp->num_orders);
            }
            else if (temp->num_orders == 1) {
                printf("%s\t\tBUY %d @ $%d (%d order)\n", LOG_PREFIX, 
                        temp->quantity, temp->price, temp->num_orders);
            }
            temp = temp->next;
        }
    }
}

// print positions of each trader
void print_positions(int num_traders, int num_products, 
                     struct trader** trader_list) {
    printf("%s\t--POSITIONS--\n", LOG_PREFIX);
    for (int i = 0; i < num_traders; i++) {
        printf("%s\tTrader %d: ", LOG_PREFIX, i);
        for (int j = 0; j < num_products; j++) {
            printf("%s %d ($%lld)", 
                    trader_list[i]->trader_prod_info[j]->product, 
                    trader_list[i]->trader_prod_info[j]->quantity, 
                    trader_list[i]->trader_prod_info[j]->balance);
            if (j != num_products - 1) {
                printf(", ");
            }
        }
        printf("\n");
    }
}

// calls orderbook and positions after each successful message from trader
void print_bk_pos(int num_products, struct printed_orderbook** printed_orders, 
                  int num_traders, struct trader** trader_list) {
    print_orderbook(num_products, printed_orders);
    print_positions(num_traders, num_products, trader_list);
}

// print that a match has happened
void print_match(struct order* new_order, struct order* old_order, 
                 long long total_price, long long fee) {
    printf("%s Match: Order %d [T%d], New Order %d [T%d], ", LOG_PREFIX, old_order->order_ID, old_order->trader_ID, new_order->order_ID, new_order->trader_ID);
    printf("value: $%lld, fee: $%lld.\n", total_price, fee);
    exchange_fees += fee; // add to total exchange fees
}

