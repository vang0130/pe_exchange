#include "pe_setup.h"

/////////////////////////////// GLOBAL VARIABLES ///////////////////////////////

long long exchange_fees = 0; // exchange fees

///////////////////////////// SMALL FUNCTIONS //////////////////////////////////

// checks if a product name is alphanumeric
int is_alphanumeric(char* line) {
    for (int i = 0; i < strlen(line) - 1; i++) {
        if (isalnum(line[i]) == 0) {
            return 0;
        }
    }
    return 1;
}

// checks if string is a number
int is_num(char* str) {
    for (int i = 0; i < strlen(str) - 1; i++) {
        if (isdigit(str[i]) == 0) {
            return 0;
        }
    }
    return 1;
}

// rounds a double to an integer
long long round_int(long double n) {
    long long x = round(n);
    return x;
}


/////////////////////////// FREEING FUNCTIONS //////////////////////////////////

// frees the order list (buy and sell for each product)
void free_orders(struct orderbook** buy_orders, struct orderbook** sell_orders, 
                 int num_products) {
    for (int i = 0; i < num_products; i++) { // for all products
        // buy orders
        struct order* temp = buy_orders[i]->head;
        struct order* next;
        while (temp != NULL) {
            next = temp->next;
            free(temp);
            temp = next;
        }
        // sell orders
        temp = sell_orders[i]->head;
        while (temp != NULL) {
            next = temp->next;
            free(temp);
            temp = next;
        }
        free(buy_orders[i]);
        free(sell_orders[i]);
    }
}

// frees the printed orders (linked list)
void free_printed_orders(struct printed_order* head) {
    struct printed_order* curr = head;
    struct printed_order* next;
    while (curr != NULL) {
        next = curr->next;
        free(curr);
        curr = next;
    }
}

// frees all memory
void free_all(struct trader** trader_list, int num_products, 
              struct printed_orderbook** printed_orders, int num_traders, 
              struct orderbook** buy_orders, 
              struct orderbook** sell_orders, char (*valid_products)[MAX_PROD],
              struct epoll_event* all_events) {
    free(all_events);
    // for each trader
    for (int i = 0; i < num_traders; i++) {
        for (int j = 0; j < num_products; j++) {
            // free balances for each product
            free(trader_list[i]->trader_prod_info[j]);
        }
        free(trader_list[i]->trader_prod_info);
        free(trader_list[i]); // free each trader
    }
    // free all orders for each product
    free_orders(buy_orders, sell_orders, num_products);
    free(buy_orders);
    free(sell_orders);

    // free_printed_orders
    for (int i = 0; i < num_products; i++) {
        free_printed_orders(printed_orders[i]->head_buy);
        free_printed_orders(printed_orders[i]->head_sell);
        free(printed_orders[i]);
    }
    free(valid_products);
    free(printed_orders); // free entire product list
    free(trader_list); // free entire trader list
}

// frees all memory
void free_early(struct trader** trader_list, int num_products, 
                char (*valid_products)[MAX_PROD], int num_traders) {
    free(valid_products);
    for (int i = 0; i < num_traders; i++) {
        free(trader_list[i]);
    }
    free(trader_list); // free entire trader list
}


//////////////////////////// CLOSING DOWN FUNCTIONS ////////////////////////////

// when a trader sends sigchld, or closes down the pipe they write to
void trader_died(int poll_rd, int i, struct trader* curr_trader, 
                 struct epoll_event* curr_event) {
    curr_trader->alive = 0; // set trader dead
    printf("%s Trader %d disconnected\n", LOG_PREFIX, i); // print message
    // delete pipe from epoll
    epoll_ctl(poll_rd, EPOLL_CTL_DEL, curr_trader->tr_fd, curr_event);
    // unlike and close the pipes
    unlink(curr_trader->ex_name);
    unlink(curr_trader->tr_name);
    close(curr_trader->ex_fd);
    close(curr_trader->tr_fd);
}

// once all traders have left the exchange
void all_traders_died(struct trader** trader_list, int num_products, 
                      struct printed_orderbook** printed_orders, 
                      int num_traders, struct orderbook** buy_orders, 
                      struct orderbook** sell_orders, 
                      char (*valid_products)[MAX_PROD], 
                      struct epoll_event* all_events) {
    // free all memory
    free_all(trader_list, num_products, printed_orders, num_traders, 
             buy_orders, sell_orders, valid_products, all_events);
    printf("%s Trading completed\n", LOG_PREFIX);
    printf("%s Exchange fees collected: $%lld\n", LOG_PREFIX, exchange_fees);
    exit(0);
}


///////////////////////// INITIALISATION FUNCTIONS /////////////////////////////

// create the printed orderbook list
// we have a list of printed orderbooks, one for each product
// each struct has a buy and sell head (linked list)
void initialise_product_list(int num_products, char (*valid_products)[MAX_PROD], 
                             struct printed_orderbook** printed_orders) {
    for (int i = 0; i < num_products; i++) {
        struct printed_orderbook* curr_prod = calloc(1, 
                                              sizeof(struct printed_orderbook));
        strncpy(curr_prod->product, valid_products[i], 16);
        curr_prod->head_buy = NULL;
        curr_prod->head_sell = NULL;
        curr_prod->buy_len = 0;
        curr_prod->sell_len = 0;
        printed_orders[i] = curr_prod;
    }
}

// initialise every trader
void initialise_trader(struct trader* curr_trader, int i, int fds[2], 
                       char buffer_ex[MAX_LINE], char buffer_tr[MAX_LINE], 
                       int num_products, int fork_res) {
    curr_trader->ID = i; // trader id
    curr_trader->order_ID = 0; // current order id (will increment)
    strcpy(curr_trader->ex_name, buffer_ex); // pipe names
    strcpy(curr_trader->tr_name, buffer_tr);
    curr_trader->trader_prod_info = NULL;
    curr_trader->ex_fd = fds[0]; // file descriptors
    curr_trader->tr_fd = fds[1];
    curr_trader->pid = fork_res; // pid
    curr_trader->alive = 1; // initially alive
}

// initialise the orderbooks for each product
void initialise_prod_orders(struct orderbook** buy_orders, 
                            struct orderbook** sell_orders, int num_products) {
    // for each product, we have a buy and sell list
    for (int i = 0; i < num_products; i++) {
        struct orderbook* curr_sell_list = calloc(1, sizeof(struct orderbook));
        struct orderbook* curr_buy_list = calloc(1, sizeof(struct orderbook));
        curr_buy_list->head = NULL;
        curr_buy_list->tail = NULL;
        curr_sell_list->head = NULL;
        curr_sell_list->tail = NULL;
        buy_orders[i] = curr_buy_list;
        sell_orders[i] = curr_sell_list;
    }
}

// initialising the inventory of each trader
void initialise_trader_inventory(int num_traders, int num_products, 
                                 char (*valid_products)[MAX_PROD], 
                                 struct trader** trader_list) {
    for (int i = 0; i < num_traders; i++) {
        trader_list[i]->trader_prod_info = calloc(num_products, 
                                           sizeof(struct trader_balances*));
        for (int j = 0; j < num_products; j++) {
            // balance and quantity for each product in the exchange
            struct trader_balances* curr_prod_info = calloc(1, sizeof(struct 
                                                     trader_balances));
            // keep the product name
            strncpy(curr_prod_info->product, valid_products[j], 16);
            curr_prod_info->balance = 0;
            curr_prod_info->quantity = 0;
            trader_list[i]->trader_prod_info[j] = curr_prod_info;
        }
    }
}


////////////////////////////// SETUP FUNCTIONS /////////////////////////////////

// fork for each trader, and create pipes
int fork_and_pipes(int num_traders, char** argv, 
                    struct trader** trader_list, int num_products, 
                    int* pids) {
    for (int i = 0; i < num_traders; i++) {
        // names of pipes to be stored here
        char buffer_ex[MAX_LINE];
        char buffer_tr[MAX_LINE];
        // create pipe name strings
        sprintf(buffer_ex, FIFO_EXCHANGE, i);
        sprintf(buffer_tr, FIFO_TRADER, i);
        // open up each pipe
        if (mkfifo(buffer_ex, 00777) < 0) {
            perror("Can't create exchange pipe");
            return i;
        }
        if (mkfifo(buffer_tr, 00777) < 0) {
            perror("Can't create trader pipe");
            return i;
        }

        int fork_res = fork();
        if (fork_res < 0) {
            perror("Failed to fork");
            return i;
        }
        else if (fork_res == 0) {
            char curr_ID[MAX_LINE];
            sprintf(curr_ID, "%d", i);
            // start each child process
            if (execlp(argv[2 + i], argv[2 + i], curr_ID, (char*)NULL) == -1) {
                perror("Execlp on trader failed");
                return i;
            }
        }
        // store the file descriptors from opening pipes
        int curr_fds[2];
        if ((curr_fds[0] = open(buffer_ex, O_WRONLY)) < 0) {
            perror("Can't open exchange pipe");
            return i;
        }
        if ((curr_fds[1] = open(buffer_tr, O_RDONLY)) < 0) {
            perror("Can't open trader pipe");
            return i;
        }
        
        // fill struct
        struct trader* curr_trader = calloc(1, sizeof(struct trader));
        initialise_trader(curr_trader, i, curr_fds, buffer_ex, 
                          buffer_tr, num_products, fork_res);
        trader_list[i] = curr_trader;
        pids[i] = fork_res;
        // print messages
        print_trader_starting(curr_trader, argv[2 + i]);
    }
    return -1;
}

// set up epoll to watch for EPOLLHUP
void create_epoll(int num_traders, struct trader** trader_list, int* poll_rd) {
    for (int i = 0; i < num_traders; i++) {
        // epoll for each trader
        struct epoll_event curr_event = {0};
        curr_event.events = EPOLLHUP; // watch for input
        curr_event.data.u32 = i; // index of trader in list
        // add fd to watch list in epoll
        epoll_ctl(*poll_rd, EPOLL_CTL_ADD, trader_list[i]->tr_fd, &curr_event);
    }
}

// only read the number of products from the file
int get_num_products(char* filename) {
    // get the product file
    FILE* product_fp = fopen(filename, "r");
    if (product_fp == NULL) {
        printf("Error opening file\n");
        return -1;
    }
    // read the first line, for number of products
    char line[MAX_LINE];
    fgets(line, MAX_LINE, product_fp);
    // CHECK if num products is int
    if (is_num(line) == 0) {
        printf("Invalid number of products\n");
        return -1;
    }
    int num_products = atoi(line);
    fclose(product_fp);
    return num_products;
}

// getting all product names from file
int get_products(char* filename, int num_products, char (*valid_products)[MAX_PROD]) {
    // get the product file
    FILE* product_fp = fopen(filename, "r");
    if (product_fp == NULL) {
        printf("Error opening file\n");
        return 1;
    }
    // read the first line, for number of products
    char line[MAX_LINE];
    fgets(line, MAX_LINE, product_fp);

    // make array of valid products
    int prod_counter = 0;
    while (fgets(line, MAX_LINE, product_fp) != NULL) {

        if (strlen(line) > MAX_PROD) {
            printf("Product name too long\n");
            fclose(product_fp);
            return 1;
        }
        if (is_alphanumeric(line) == 0) {
            printf("Invalid product name\n");
            fclose(product_fp);
            return 1;
        }
        prod_counter++;
        // slice off newline
        line[strlen(line) - 1] = '\0';
        // add to array
        strcpy(valid_products[prod_counter - 1], line);
        if (prod_counter > num_products) {
            printf("Too many products for number given\n");
            fclose(product_fp);
            return 1;
        }
    }
    if (prod_counter < num_products) {
        printf("Not enough products for number given\n");
        fclose(product_fp);
        return 1;
    }
    fclose(product_fp);
	return 0;
}

