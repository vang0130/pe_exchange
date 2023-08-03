#ifndef PE_TRADER_H
#define PE_TRADER_H

#include "pe_common.h"

// GLOBALS
int signal_received;
int trader_order_ID;

// FUNCTION DECLARATIONS
void (handler)();

int process_msg(int fd, char* msg);

int strlen_digit(int x);

int find_buy_str_len(struct order* curr_order);

int create_buy_order(char* sell_str, struct order* curr_buy);

char* create_buy_str(struct order* curr_order);

void write_and_kill(int fd, char* msg, pid_t exchange_pid);

#endif
