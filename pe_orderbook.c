#include "pe_orderbook.h"

////////////////////////// CREATING ORDER FUNCTIONS ////////////////////////////

// creates a new buy or sell order
struct order* create_order(char* str, int ind, struct trader* curr_trader, 
                           char (*valid_products)[MAX_PROD], int num_products,
                           int is_buy) {
    int order_ID = 0; // get information from parsed message
    int quantity = 0;
    int price = 0;
    char product[MAX_PROD];

    int ret = sscanf(str, "%*s %d %s %d %d;", &order_ID, product,
                     &quantity, &price);
    if (ret != 4) {
        return NULL;
    }
    // find the product index of the order
    int found = 0;
    int prod_ind = 0;
    for (prod_ind = 0; prod_ind < num_products; prod_ind++) {
        if (strcmp(product, valid_products[prod_ind]) == 0) {
            found = 1;
            break;
        }
    }
    if (found == 0) {
        return NULL;
    }
    // error checking on values
    if (quantity > 999999 || quantity < 1) {
        return NULL;
    }
    if (order_ID != curr_trader->order_ID || 
        order_ID < 0 || order_ID > 999999) {
        return NULL;
    }
    if (price > 999999 || price < 1) {
        return NULL;
    }
    // creating order struct
    curr_trader->order_ID++;
    struct order* curr_order = calloc(1, sizeof(struct order));
    curr_order->order_ID = order_ID;
    curr_order->product_ID = prod_ind;
    curr_order->quantity = quantity;
    curr_order->price = price;
    curr_order->is_buy = is_buy;
    curr_order->trader_ID = curr_trader->ID;

    strcpy(curr_order->product, product);
    curr_order->is_buy = is_buy;
    return curr_order;
}

// checks if order is buy or sell, sends off to function to create
struct order* create_buy_sell_order(char* str, int ind, 
                                    struct trader* curr_trader, 
                                    char (*valid_products)[MAX_PROD], 
                                    int num_products) {
    // send off to either create buy or sell struct
    char sell_or_buy[MAX_LINE];
    sscanf(str, "%s %*d %*s %*d %*d;", sell_or_buy);
    if (strcmp(sell_or_buy, "SELL") == 0) {
        struct order* curr_order = create_order(str, ind, curr_trader, 
                                                valid_products, 
                                                num_products, 0);
        if (curr_order == NULL) {
            return NULL;
        }
        return curr_order;
    }
    else if (strcmp(sell_or_buy, "BUY") == 0) {
        struct order* curr_order = create_order(str, ind, curr_trader, 
                                                valid_products, 
                                                num_products, 1);
        if (curr_order == NULL) {
            return NULL;
        }
        return curr_order;
    }
    else {
        return NULL;
    }
}

// create an order to print
struct printed_order* create_printed_order(struct order* curr_order) {
    struct printed_order* new_order = calloc(sizeof(struct printed_order), 1);
    new_order->quantity = curr_order->quantity;
    new_order->price = curr_order->price;
    new_order->num_orders = 1; // number of levels
    new_order->next = NULL;
    return new_order;
}

// create a new order once order has been amended
struct order* create_amended_order(int new_price, int new_quantity, 
                                   struct order* curr_order) {
    struct order* new_order = calloc(1, sizeof(struct order));
    new_order->price = new_price;
    new_order->quantity = new_quantity;
    new_order->is_buy = curr_order->is_buy;
    new_order->order_ID = curr_order->order_ID;
    new_order->trader_ID = curr_order->trader_ID;
    new_order->product_ID = curr_order->product_ID;
    strcpy(new_order->product, curr_order->product);
    new_order->next = NULL;
    new_order->prev = NULL;
    return new_order;
}

// check the number of buy and sell levels in the printed orderbook
void set_len(struct printed_orderbook* curr_prod, int is_buy) {
    int ctr = 0;
    if (is_buy == 1) {
        struct printed_order* temp = curr_prod->head_buy;
        while (temp != NULL) {
            ctr++;
            temp = temp->next;
        }
        curr_prod->buy_len = ctr;
    }
    else {
        struct printed_order* temp = curr_prod->head_sell;
        while (temp != NULL) {
            ctr++;
            temp = temp->next;
        }
        curr_prod->sell_len = ctr;
    }
}

//////////////////////////////// ADDING TO LISTS ///////////////////////////////

// adds something to products list
void add_to_prints(struct printed_orderbook* curr_prod, 
                   struct order* curr_order, int is_buy) {
    if (is_buy == 1) {
        // if only head or nothing in list
        if (curr_prod->head_buy == NULL || 
            curr_prod->head_buy->price < curr_order->price) {
            struct printed_order* new_order = create_printed_order(curr_order);
            new_order->next = curr_prod->head_buy;
            curr_prod->head_buy = new_order;
            return;
        }
        // search until we find an order with price just higher or equal to curr order
        struct printed_order* temp = curr_prod->head_buy;
        while (temp->next != NULL && temp->next->price >= curr_order->price) {
            temp = temp->next;
        }
        if (temp->price == curr_order->price) { // if price is equal
            temp->quantity += curr_order->quantity; // increment quantity
            temp->num_orders++; // increment number of orders
            return;
        }
        else if (temp->price > curr_order->price) { // create new order and add
            struct printed_order* new_order = create_printed_order(curr_order);
            new_order->next = temp->next;
            temp->next = new_order;
            return;
        }
    }
    else {
        // add to sell order
        if (curr_prod->head_sell == NULL || 
            curr_prod->head_sell->price < curr_order->price) {
            struct printed_order* new_order = create_printed_order(curr_order);
            new_order->next = curr_prod->head_sell;
            curr_prod->head_sell = new_order;
            return;
        }
        struct printed_order* temp = curr_prod->head_sell;
        while (temp->next != NULL && temp->next->price >= curr_order->price) {
            temp = temp->next;
        }
        if (temp->price == curr_order->price) {
            temp->quantity += curr_order->quantity;
            temp->num_orders++;
            return;
        }
        else if (temp->price > curr_order->price) {
            struct printed_order* new_order = create_printed_order(curr_order);
            new_order->next = temp->next;
            temp->next = new_order;
            return;
        }
    }
}

// sends off to add an order to the printed orderbook, and sets the new buy/sell level
void add_order_prints(struct printed_orderbook** printed_orders, 
                      struct order* curr_order) {
    if (curr_order->is_buy == 1) {
        add_to_prints(printed_orders[curr_order->product_ID], curr_order, 1);
        set_len(printed_orders[curr_order->product_ID], 1);
    }
    else {
        add_to_prints(printed_orders[curr_order->product_ID], curr_order, 0);
        set_len(printed_orders[curr_order->product_ID], 0);
    }
}

// adds a buy/sell order to the end of the linked list
// new orders are added to tail, because list if from oldest to newest order
void add_to_tail(struct order* curr_order, struct orderbook* list) {	
    if (list->tail == NULL) {
        list->tail = curr_order;
        list->head = curr_order;
        curr_order->prev = NULL;
        curr_order->next = NULL;
    }
    else {
        curr_order->prev = list->tail;
        list->tail->next = curr_order;
        list->tail = curr_order;
        curr_order->next = NULL;
    }
}

// sends off to add_to_tail function for buy/sell orders
void add_order(struct order* curr_order, struct orderbook** buy_orders, 
               struct orderbook** sell_orders) {
    // add order to trader
    struct orderbook* list = NULL;
    if (curr_order->is_buy == 1) {
        list = buy_orders[curr_order->product_ID];
    }
    else {
        list = sell_orders[curr_order->product_ID];
    }
    add_to_tail(curr_order, list);
}


///////////////////////////// DELETING FROM LISTS //////////////////////////////

// removes a buy/sell order from the printed orderbook
void remove_from_products(struct printed_order* node, 
                          struct printed_orderbook* curr_prod, int is_buy) {
    if (is_buy == 1) { // if buy order
        // if only one order in list, set head to null
        if (curr_prod->buy_len == 1) {
            curr_prod->head_buy = NULL;
            return;
        }
        // if current node is the head, set head to next node
        else if (node == curr_prod->head_buy) {
            curr_prod->head_buy = curr_prod->head_buy->next;
            return;
        }
        // otherwise, iterate until we find the right price
        struct printed_order* temp = curr_prod->head_buy;
        while (temp->next != NULL && temp->next->price != node->price) {
            temp = temp->next;
        }
        // skip the current nope in the list
        temp->next = node->next;
    }
    else { // if sell order
        if (curr_prod->sell_len == 1) {
            curr_prod->head_sell = NULL;
            return;
        }
        else if (node == curr_prod->head_sell) {
            curr_prod->head_sell = curr_prod->head_sell->next;
            return;
        }
        struct printed_order* temp = curr_prod->head_sell;
        while (temp->next != NULL && temp->next->price != node->price) {
            temp = temp->next;
        }
        temp->next = node->next;
    }
}

// deducts current order from printed orders based on price
void delete_order_prints(struct printed_orderbook* curr_prod, 
                         struct order* curr_order) {
    // we have the product list
    // we have the price and quantity
    struct printed_order* temp = NULL;
    if (curr_order->is_buy == 1) {
        temp = curr_prod->head_buy;
        while (temp != NULL) { // iterate through linked list
            if (curr_order->price == temp->price) { // find correct price
                temp->quantity -= curr_order->quantity; // decrement quantity
                temp->num_orders--; // decrement num_orders
                if (temp->quantity == 0) {
                    // only remove if remaining quantity is 0
                    remove_from_products(temp, curr_prod, 1);
                    free(temp);
                    return;
                }
            }
            temp = temp->next;
        }
    }
    else {
        temp = curr_prod->head_sell;
        while (temp != NULL) {
            if (curr_order->price == temp->price) {
                temp->quantity -= curr_order->quantity;
                temp->num_orders--;
                if (temp->quantity == 0) {
                    remove_from_products(temp, curr_prod, 0);
                    free(temp);
                    return;
                }
            }
            temp = temp->next;
        }
    }
}

// if an order is cancelled, we need to remove it from the printed orderbook
void cancel_order_prints(struct order* curr_order, 
                         struct printed_orderbook** printed_orders) {
    delete_order_prints(printed_orders[curr_order->product_ID], curr_order);
    if (curr_order->is_buy == 1) {
        // set the buy/sell level
        set_len(printed_orders[curr_order->product_ID], 1);
    }
    else {
        set_len(printed_orders[curr_order->product_ID], 0);
    }
}

// deletes order from orders list
void delete_order(struct order* curr_order, struct orderbook* list) {
    // if only one node in list, set head and tail to null
    if (list->tail == list->head && list->head == curr_order) {
        list->head = NULL;
        list->tail = NULL;
    }
    // if current order is the head, set head to next node
    else if (list->head == curr_order) {
        list->head = curr_order->next;
        list->head->prev = NULL;
    }
    // if current order is the tail, set tail to prev node
    else if (list->tail == curr_order) {
        list->tail = curr_order->prev;
        list->tail->next = NULL;
    }
    // otherwise, skip the current node
    else {
        curr_order->prev->next = curr_order->next;
        curr_order->next->prev = curr_order->prev;
    }
}

// cancels an order from the orderbook
int cancel_order(struct order* curr_order, struct orderbook* list) {
    // pop from list
    delete_order(curr_order, list);
    free(curr_order);

    return 1;
}

////////////////////////////// LOCATING ORDERS /////////////////////////////////

// returns a pointer to an order
// searches for a matching id
struct order* find_order(int num_products, int trader_id, int id, 
                         struct orderbook** buy_orders, 
                         struct orderbook** sell_orders) {
    struct order* curr_order = NULL;
    // iterate through products
    for (int i = 0; i < num_products; i++) {
        curr_order = buy_orders[i]->head;
        while (curr_order != NULL) {
            // match the trader, and match the current order id
            if (curr_order->order_ID == id && 
                curr_order->trader_ID == trader_id) {
                return curr_order;
            }
            curr_order = curr_order->next;
        }
        curr_order = sell_orders[i]->head;
        while (curr_order != NULL) {
            if (curr_order->order_ID == id && 
                curr_order->trader_ID == trader_id) {
                return curr_order;
            }
            curr_order = curr_order->next;
        }
    }
    // if not found, return NULL
    return NULL;
}

// find the product list that an order belongs in
struct orderbook* find_list(struct order* order_to_find, 
                            struct orderbook** buy_orders, 
                            struct orderbook** sell_orders) {
    if (order_to_find->is_buy == 1) {
        return buy_orders[order_to_find->product_ID];
    }
    return sell_orders[order_to_find->product_ID];
}


////////////////////////////// MATCHING FUNCTIONS //////////////////////////////

// update the product inventory of buy and sell traders after each match
void update_inventory(struct trader* buy_trader, struct trader* sell_trader, 
                      long long total_price, int quantity, int prod_ind) {
    buy_trader->trader_prod_info[prod_ind]->quantity += quantity;
    buy_trader->trader_prod_info[prod_ind]->balance -= total_price;
    sell_trader->trader_prod_info[prod_ind]->quantity -= quantity;
    sell_trader->trader_prod_info[prod_ind]->balance += total_price;
}

// find the cheapest and oldest sell order for relevant product
// match with new buy order
struct order* find_best_sell(struct order* buy_order, 
                             struct orderbook** sell_orders) {
    struct order* curr_order = sell_orders[buy_order->product_ID]->head;
    if (curr_order == NULL) {
        return NULL;
    }
    struct order* best_order = curr_order;
    int min_price = curr_order->price;
    while (curr_order != NULL) {
        if (curr_order->price < min_price) { // only take if strictly lesser
            min_price = curr_order->price;
            best_order = curr_order;
        }
        curr_order = curr_order->next;
    }
    if (min_price <= buy_order->price) {
        return best_order;
    }
    return NULL;
}

// find the most expensive and oldest buy order for relevant product
struct order* find_best_buy(struct order* sell_order, 
                            struct orderbook** buy_orders) {
    struct order* curr_order = buy_orders[sell_order->product_ID]->head;
    // if order can match, it will be in this list
    int max_price = 0;
    struct order* best_order = NULL;
    while (curr_order != NULL) {
        if (curr_order->price > max_price) { // only take if strictly greater
            max_price = curr_order->price;
            best_order = curr_order;
        }
        curr_order = curr_order->next;
    }
    if (max_price >= sell_order->price) {
        return best_order;
    }
    return NULL;
}

// if new buy order, send here to match
int match_new_buy(struct trader** trader_list, struct order* buy_order, 
                  struct order* sell_order, 
                  struct printed_orderbook** printed_orders, 
                  struct orderbook** buy_orders, 
                  struct orderbook** sell_orders) {
    struct trader* buy_trader = trader_list[buy_order->trader_ID];
    struct trader* sell_trader = trader_list[sell_order->trader_ID];
    int prod_ind = buy_order->product_ID;
    long long total_price = 0;
    long long fee = 0;
    // if quantity is equal
    if (buy_order->quantity == sell_order->quantity) {
        // write filled to the two traders
        write_fill(buy_order, sell_order, sell_order->quantity, 
                   buy_trader, sell_trader);
        // calculate total price, type long long to avoid overflow
        total_price = (long long)sell_order->price * 
                      (long long)sell_order->quantity;
        fee = round_int(total_price * FEE_PERCENTAGE * 0.01); // calculate fee to nearest int
        // decrement fee from newer trader
        buy_trader->trader_prod_info[prod_ind]->balance -= fee;
        // update inventory of both buyers for current product
        update_inventory(buy_trader, sell_trader, total_price, 
                         sell_order->quantity, prod_ind);
        // print that order has matched
        print_match(buy_order, sell_order, total_price, fee);
        // delete the sell order from printed and regular orderbook
        cancel_order_prints(sell_order, printed_orders);
        cancel_order(sell_order, sell_orders[prod_ind]);
        // free the buy order
        free(buy_order);
    }
    // if buyer wants more quantity than seller
    else if (buy_order->quantity > sell_order->quantity) {
        write_fill(buy_order, sell_order, sell_order->quantity, 
                   buy_trader, sell_trader);
        total_price = (long long)sell_order->price * 
                      (long long)sell_order->quantity;
        fee = round_int(total_price * 0.01);
        buy_trader->trader_prod_info[prod_ind]->balance -= fee;
        update_inventory(buy_trader, sell_trader, total_price, 
                         sell_order->quantity, prod_ind);
        print_match(buy_order, sell_order, total_price, fee);
        // delete sell order, as there is no quantity left
        cancel_order_prints(sell_order, printed_orders);
        delete_order(sell_order, sell_orders[prod_ind]);
        // update the buy order quantity
        buy_order->quantity = buy_order->quantity - sell_order->quantity;
        free(sell_order); // free the sell order
        return 1; // return 1, as there is still quantity left in buy order
    }
    else { // buy order quantity < sell order quantity
        // buy order will be gone, but sell order will remain
        write_fill(buy_order, sell_order, buy_order->quantity, 
                   buy_trader, sell_trader);
        total_price = (long long)sell_order->price * 
                      (long long)buy_order->quantity;
        fee = round_int(total_price * 0.01);
        buy_trader->trader_prod_info[prod_ind]->balance -= fee;
        update_inventory(buy_trader, sell_trader, total_price, 
                         buy_order->quantity, prod_ind);
        print_match(buy_order, sell_order, total_price, fee);
        // delete the order from print list
        cancel_order_prints(sell_order, printed_orders);
        // amend and add back in
        sell_order->quantity = sell_order->quantity - buy_order->quantity;
        add_order_prints(printed_orders, sell_order);
        free(buy_order);
    }
    return 0;
}

// if new sell order, send here to match
int match_new_sell(struct trader** trader_list, struct order* sell_order, 
                   struct order* buy_order, 
                   struct printed_orderbook** printed_orders, 
                   struct orderbook** buy_orders, 
                   struct orderbook** sell_orders) {
    struct trader* buy_trader = trader_list[buy_order->trader_ID];
    struct trader* sell_trader = trader_list[sell_order->trader_ID];
    int prod_ind = sell_order->product_ID;
    int total_price = 0;
    int fee = 0;
    if (buy_order->quantity == sell_order->quantity) {
        write_fill(buy_order, sell_order, sell_order->quantity, 
                   buy_trader, sell_trader);
        total_price = (long long)buy_order->price * 
                      (long long)sell_order->quantity;
        fee = round_int(total_price * 0.01);
        sell_trader->trader_prod_info[prod_ind]->balance -= fee;
        update_inventory(buy_trader, sell_trader, total_price, 
                         sell_order->quantity, prod_ind);
        print_match(sell_order, buy_order, total_price, fee);
        cancel_order_prints(buy_order, printed_orders);
        delete_order(buy_order, buy_orders[prod_ind]);
        free(buy_order);
        free(sell_order);
    }
    else if (buy_order->quantity > sell_order->quantity) {
        write_fill(buy_order, sell_order, sell_order->quantity, 
                   buy_trader, sell_trader);
        total_price = (long long)buy_order->price * 
                      (long long)sell_order->quantity;
        fee = round_int(total_price * 0.01);
        sell_trader->trader_prod_info[prod_ind]->balance -= fee;
        update_inventory(buy_trader, sell_trader, total_price, 
                         sell_order->quantity, prod_ind);
        print_match(sell_order, buy_order, total_price, fee);
        // delete the order from print list
        cancel_order_prints(buy_order, printed_orders);
        // amend and add back in
        buy_order->quantity = buy_order->quantity - sell_order->quantity;
        add_order_prints(printed_orders, buy_order);
        free(sell_order);
    }
    else { // buy order quantity < sell order quantity
        write_fill(buy_order, sell_order, buy_order->quantity, 
                   buy_trader, sell_trader);
        total_price = (long long)buy_order->price * 
                      (long long)buy_order->quantity;
        fee = round_int(total_price * 0.01);
        sell_trader->trader_prod_info[prod_ind]->balance -= fee;
        update_inventory(buy_trader, sell_trader, total_price, 
                         buy_order->quantity, prod_ind);
        print_match(sell_order, buy_order, total_price, fee);
        cancel_order_prints(buy_order, printed_orders);
        delete_order(buy_order, buy_orders[prod_ind]);
        sell_order->quantity = sell_order->quantity - buy_order->quantity;
        free(buy_order);
        return 1;
    }
    return 0;
}

// checks if match is possible after new/amended buy/sell order
int check_for_match(struct order* new_order, struct orderbook** buy_orders, 
                    struct orderbook** sell_orders, 
                    struct trader** trader_list, 
                    struct printed_orderbook** printed_orders) {
    // find the correct product	
    int order_gone = 1;
    if (new_order->is_buy == 1) {
        while (new_order->quantity > 0) {
            struct order* matched_sell = find_best_sell(new_order, sell_orders);
            if (matched_sell == NULL) {
                // no match possible
                return 1;
            }
            // call a function to do the matching
            int ret = match_new_buy(trader_list, new_order, matched_sell, 
                                    printed_orders, buy_orders, sell_orders);
            // if any quantity left, call again
            if (ret == 0) {
                order_gone = 0;
                break;
            }
        }
        // if some of the new order remains, 
        // we have to add it to orderbook and printed orderbook lists
        if (order_gone == 1) {
            // add new order to lists
            return 1;
        }
    }
    else {
        while (new_order->quantity > 0) {
            struct order* matched_buy = find_best_buy(new_order, buy_orders);
            if (matched_buy == NULL) {
                // no match possible
                return 1;
            }
            int ret = match_new_sell(trader_list, new_order, matched_buy, 
                                     printed_orders, buy_orders, sell_orders);
            if (ret == 0) {
                order_gone = 0;
                break;
            }
        }
        if (order_gone == 1) {
            // add new order to lists
            return 1;
        }
    }
    return 0;
}
