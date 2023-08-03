#include "pe_pipes.h"

//////////////////////////// WRITING FUNCTIONS /////////////////////////////////

// writes to a trader and sends SIGUSR1
void write_and_kill(int fd, char msg[MAX_LINE], pid_t pid) {
    write(fd, msg, strlen(msg));
    kill(pid, SIGUSR1);
}

// writes an invalid message to trader
void write_invalid(struct trader* curr_trader) {
    char msg[MAX_LINE] = {0};
    strcpy(msg, "INVALID;");
    write_and_kill(curr_trader->ex_fd, msg, curr_trader->pid);
}

// writes an accepted message to a trader after valid buy/sell
// also writes a market message to all other traders
void write_accepted(struct order* curr_order, int trader_ind, 
                    struct trader** trader_list, int num_traders) {
    char msg[MAX_LINE];
    sprintf(msg, "ACCEPTED %d;", curr_order->order_ID);
    write_and_kill(trader_list[trader_ind]->ex_fd, msg, 
                   trader_list[trader_ind]->pid);
    memset(msg, 0, 1);
    if (curr_order->is_buy == 1) {
        sprintf(msg, "MARKET BUY %s %d %d;", curr_order->product, 
                curr_order->quantity, curr_order->price);
    }
    else {
        sprintf(msg, "MARKET SELL %s %d %d;", curr_order->product, 
                curr_order->quantity, curr_order->price);
    }
    // sending to all other traders
    for (int i = 0; i < num_traders; i++) {
        if (i != trader_ind && trader_list[i]->alive != 0) {
            write_and_kill(trader_list[i]->ex_fd, msg, trader_list[i]->pid);
        }
    }
}

// writes amended message after successful amend
// writes market to all other traders
void write_amended(struct order* curr_order, int trader_ind, 
                   struct trader** trader_list, int num_traders) {
    char msg[MAX_LINE];
    sprintf(msg, "AMENDED %d;", curr_order->order_ID);
    write_and_kill(trader_list[trader_ind]->ex_fd, msg, 
                   trader_list[trader_ind]->pid);
    memset(msg, 0, 1);
    if (curr_order->is_buy == 1) {
        sprintf(msg, "MARKET BUY %s %d %d;", curr_order->product, 
                curr_order->quantity, curr_order->price);
    }
    else {
        sprintf(msg, "MARKET SELL %s %d %d;", curr_order->product, 
                curr_order->quantity, curr_order->price);
    }
    for (int i = 0; i < num_traders; i++) {
        if (i != trader_ind && trader_list[i]->alive != 0) {
            write_and_kill(trader_list[i]->ex_fd, msg, trader_list[i]->pid);
        }
    }
}

// write cancelled message after successful cancel
// write market to all other traders, with price and quantity = 0
void write_cancelled(int id, char product[MAX_PROD], int trader_ind, 
                     struct trader** trader_list, int num_traders, int is_buy) {
    char msg[MAX_LINE];
    sprintf(msg, "CANCELLED %d;", id);
    write_and_kill(trader_list[trader_ind]->ex_fd, msg, 
                   trader_list[trader_ind]->pid);
    memset(msg, 0, 1);
    if (is_buy == 1) {
        sprintf(msg, "MARKET BUY %s 0 0;", product);
    }
    else {
        sprintf(msg, "MARKET SELL %s 0 0;", product);
    }
    for (int i = 0; i < num_traders; i++) {
        if (i != trader_ind && trader_list[i]->alive != 0) {
            write_and_kill(trader_list[i]->ex_fd, msg, trader_list[i]->pid);
        }
    }
}

// write fill message after successful match
// write to buy trader first, then sell trader
void write_fill(struct order* buy_order, struct order* sell_order, int quantity, 
                struct trader* buy_trader, struct trader* sell_trader) {
    char msg[MAX_LINE];
    sprintf(msg, "FILL %d %d;", buy_order->order_ID, quantity);
    if (buy_trader->alive != 0) {
        write_and_kill(buy_trader->ex_fd, msg, buy_trader->pid);
    }
    sprintf(msg, "FILL %d %d;", sell_order->order_ID, quantity);
    if (sell_trader->alive != 0) {
        write_and_kill(sell_trader->ex_fd, msg, sell_trader->pid);
    }
}

// write market open to all traders once exchange starts
void write_market_open(struct trader** trader_list, int num_traders) {
    char msg[MAX_LINE];
    strcpy(msg, "MARKET OPEN;");
    for (int i = 0; i < num_traders; i++) {
        if (trader_list[i]->alive != 0) {
            write_and_kill(trader_list[i]->ex_fd, msg, trader_list[i]->pid);
        }
    }
}

//////////////////////////// PARSING MESSAGE FUNCTIONS /////////////////////////

int process_msg(int fd, char* msg) {
    int reading = 1;
    int count = 0;
    while (reading == 1) {
        char c;
        int num = read(fd, &c, 1);
        if (num == -1) {
            if (errno == EINTR) {
                break;
            }
            else {
                perror("Reading error: ");
                return 1;
            }
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
    return count;
}

// when buy or sell message is received 
int buy_sell_msg(char msg[MAX_LINE], int trader_ind, 
                struct trader** trader_list, int num_traders, 
                struct printed_orderbook** printed_orders, 
                char (*valid_products)[MAX_PROD], int num_products, 
                struct orderbook** buy_orders, 
                struct orderbook** sell_orders) {
    int space_count = 0; // check number of spaces in msg
    for (int i = 0; i < strlen(msg); i++) {
        if (msg[i] == ' ') {
            space_count++;
        }
    }
    if (space_count != 4) {
        return 0; // send invalid msg to trader
    }
    // create the order
    struct order* curr_order = create_buy_sell_order(msg, trader_ind, 
                                                     trader_list[trader_ind], 
                                                     valid_products, 
                                                     num_products);
    if (curr_order == NULL) { // if msg was not valid
        return 0; // send invalid msg to trader
    }
    // write accepted to trader, and send market to all others
    write_accepted(curr_order, trader_ind, trader_list, num_traders);
    // check if match possible
    int ret = check_for_match(curr_order, buy_orders, sell_orders, 
                              trader_list, printed_orders);
    // if some or all of the new order remains, add it to the orderbooks
    if (ret == 1) {
        add_order_prints(printed_orders, curr_order);
        add_order(curr_order, buy_orders, sell_orders);
    }
    // print orderbook and positions
    print_bk_pos(num_products, printed_orders, num_traders, trader_list);
    return 1;
}

// checks if an amend msg is valid
struct order* check_valid_amend(int quantity, int price, int num_products, 
                                int trader_ind, 
                                struct orderbook** buy_orders, 
                                struct orderbook** sell_orders, 
                                char msg[MAX_LINE]) {
    int space_count = 0;
    for (int i = 0; i < strlen(msg); i++) {
        if (msg[i] == ' ') {
            space_count++;
        }
    }
    if (space_count != 3) {
        return NULL; // if msg is not valid
    }
    
    int id = 0;
    sscanf(msg, "AMEND %d %d %d", &id, &quantity, &price);
    // further validity checking of msg
    if (quantity < 1 || quantity > 999999) {
        return NULL;
    }
    if (price < 1 || price > 999999) {
        return NULL;
    }
    if (id < 0 || id > 999999) {
        return NULL;
    }
    // finds the current order in the orderbook based on trader and orderID and returns
    struct order* curr_order = find_order(num_products, trader_ind, id, 
                                          buy_orders, sell_orders);
    return curr_order;
}

// when amend message is received
int amend_msg(char msg[MAX_LINE], int trader_ind, struct trader** trader_list, 
              int num_traders, struct printed_orderbook** printed_orders,
              int num_products, struct orderbook** buy_orders, 
              struct orderbook** sell_orders) {
    int new_quantity = 0;
    int new_price = 0;
    int num_scanned = sscanf(msg, "AMEND %*d %d %d", &new_quantity, &new_price);
    if (num_scanned != 2) {
        return 0;
    }
    // check valid and get the order from lists
    struct order* curr_order = check_valid_amend(new_quantity, new_price, 
                                                 num_products, trader_ind, 
                                                 buy_orders, sell_orders, msg);
    if (curr_order == NULL) {
        return 0;
    }
    // create the new amended order
    struct order* new_order = create_amended_order(new_price, new_quantity, 
                                                   curr_order);
    write_amended(new_order, trader_ind, trader_list, num_traders);
    struct orderbook* curr_list = find_list(curr_order, buy_orders, 
                                            sell_orders);
    // delete the old order from everything
    cancel_order_prints(curr_order, printed_orders);
    delete_order(curr_order, curr_list);
    free(curr_order);
    // check if a match is possible with the amended order
    int ret = check_for_match(new_order, buy_orders, sell_orders, 
                              trader_list, printed_orders);
    // if anything left, add back to products
    if (ret == 1) {
        // amended order gets added to the tail of the orderbook (updating age)
        add_order_prints(printed_orders, new_order);
        add_order(new_order, buy_orders, sell_orders);
    }
    print_bk_pos(num_products, printed_orders, num_traders, trader_list);
    return 1;
}

// when cancel message is received
int cancel_msg(char msg[MAX_LINE], int trader_ind, struct trader** trader_list, 
               int num_traders, struct printed_orderbook** printed_orders,
               int num_products, struct orderbook** buy_orders, 
               struct orderbook** sell_orders) {
    // error check here
    int space_count = 0;
    for (int i = 0; i < strlen(msg); i++) {
        if (msg[i] == ' ') {
            space_count++;
        }
    }
    if (space_count != 1) {
        return 0;
    }
    int id;
    int num_ret = sscanf(msg, "CANCEL %d", &id);
    if (num_ret != 1) { 
        return 0;
    }
    if (id < 0 || id > 999999) {
        return 0;
    }
    // find the order
    struct order* curr_order = find_order(num_products, trader_ind, 
                                          id, buy_orders, sell_orders);
    if (curr_order == NULL) {
        return 0;
    }
    write_cancelled(id, curr_order->product, trader_ind, trader_list, 
                    num_traders, curr_order->is_buy);
    struct orderbook* curr_list = find_list(curr_order, buy_orders, 
                                            sell_orders);
    // delete the order
    cancel_order_prints(curr_order, printed_orders);
    cancel_order(curr_order, curr_list);
    print_bk_pos(num_products, printed_orders, num_traders, trader_list);
    return 1;
}

// parses a trader message to the relevant function
void parse_msg(char msg[MAX_LINE], int trader_ind, struct trader** trader_list, 
               int num_traders, struct printed_orderbook** printed_orders, 
               char (*valid_products)[MAX_PROD], int num_products, 
               struct orderbook** buy_orders, 
               struct orderbook** sell_orders) {
    if (msg[0] == 'B' || msg[0] == 'S') {
        int ret = buy_sell_msg(msg, trader_ind, trader_list, num_traders, 
                               printed_orders, valid_products, num_products, 
                               buy_orders, sell_orders);
        if (ret == 0) {
            write_invalid(trader_list[trader_ind]);
        }
    }
    else if (msg[0] == 'A') {
        int ret = amend_msg(msg, trader_ind, trader_list, num_traders, 
                            printed_orders, num_products, 
                            buy_orders, sell_orders);
        if (ret == 0) {
            write_invalid(trader_list[trader_ind]);
        }
    }
    else if (msg[0] == 'C') {
        int ret = cancel_msg(msg, trader_ind, trader_list, num_traders, 
                             printed_orders, num_products, 
                             buy_orders, sell_orders);
        if (ret == 0) {
            write_invalid(trader_list[trader_ind]);
        }
    }
    else {
        write_invalid(trader_list[trader_ind]);
    }
}
