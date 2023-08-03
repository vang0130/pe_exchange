#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <stdint.h>
#include <string.h>

#include "cmocka.h"

#include "../pe_common.h"
// #include "../pe_trader.h"
#include "../pe_exchange.h"
#include "../pe_orderbook.h"
#include "../pe_pipes.h"
#include "../pe_printing.h"
#include "../pe_setup.h"


static void test_alphanumeric(void **state) {
    char str[] = "hello\n\0"; // newline because fgets reads \n
    char str1[] = "hello123\n\0";
    char str2[] = "poopoo(&(*&(*&*(!\n\0";
    assert_true(is_alphanumeric(str) == 1);
    assert_true(is_alphanumeric(str1) == 1);
    assert_true(is_alphanumeric(str2) == 0);
}


static void test_is_num(void **state) {
    char str[] = "hello\n";
    char str1[] = "123\n";
    char str2[] = ")(*)(*)\n";
    assert_true(is_num(str) == 0);
    assert_true(is_num(str1) == 1);
    assert_true(is_num(str2) == 0);
}


static void test_round_int(void **state) {
    assert_true(round_int(1.5) == 2);
    assert_true(round_int(1.4) == 1);
    assert_true(round_int(1.8) == 2);
    assert_true(round_int(19187219871239.8) == 19187219871240);
    assert_true(round_int(19187219871239.4) == 19187219871239);
}


static void test_get_num_products(void **state) {
    assert_true(get_num_products("nonexistent.txt") == -1);
    assert_true(get_num_products("./tests/prod_files/test.txt") == 2);
    assert_true(get_num_products("./tests/prod_files/not_an_int.txt") == -1);
}


static void test_get_products(void **state) {
    char (*valid_products)[MAX_PROD] = calloc(10, sizeof(char[MAX_PROD]));
    assert_true(get_products("nonexistent.txt", 1, valid_products) == 1);
    assert_true(get_products("./tests/prod_files/name_too_long.txt", 2, valid_products) == 1);
    assert_true(get_products("./tests/prod_files/not_alnum.txt", 4, valid_products) == 1);
    assert_true(get_products("./tests/prod_files/too_many.txt", 3, valid_products) == 1);
    assert_true(get_products("./tests/prod_files/too_little.txt", 3, valid_products) == 1);
    assert_true(get_products("./tests/prod_files/products.txt", 2, valid_products) == 0);
}


static void test_create_amended_order(void **state) {
    struct order* curr = calloc(1, sizeof(struct order));
    curr->order_ID = 1;
    curr->is_buy = 0;
    curr->trader_ID = 100;
    curr->product_ID = 5;
    curr->price = 100;
    curr->quantity = 100;
    strcpy(curr->product, "PRODUCT");
    curr->next = NULL;
    curr->prev = NULL;
    struct order* new = create_amended_order(500, 1000, curr);
    assert_true(new->order_ID == 1);
    assert_true(new->is_buy == 0);
    assert_true(new->trader_ID == 100); 
    assert_true(new->product_ID == 5);
    assert_true(new->price == 500);
    assert_true(new->quantity == 1000);
    assert_true(strcmp(new->product, "PRODUCT") == 0);
    assert_true(new->next == NULL);
    assert_true(new->prev == NULL);
    free(curr);
    free(new);
}

static void test_create_printed_order(void **state) {
    struct order* curr = calloc(1, sizeof(struct order));
    curr->order_ID = 1;
    curr->is_buy = 0;
    curr->trader_ID = 100;
    curr->product_ID = 5;
    curr->price = 500;
    curr->quantity = 1000;
    strcpy(curr->product, "PRODUCT");
    curr->next = NULL;
    curr->prev = NULL;
    struct printed_order* new = create_printed_order(curr);
    assert_true(new->price == 500);
    assert_true(new->quantity == 1000);
    assert_true(new->next == NULL);
    assert_true(new->num_orders == 1);
    free(curr);
    free(new);
}



int main(void) {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_alphanumeric),
        cmocka_unit_test(test_is_num),
        cmocka_unit_test(test_round_int),
        cmocka_unit_test(test_get_num_products),
        cmocka_unit_test(test_get_products),
        cmocka_unit_test(test_create_amended_order),
        cmocka_unit_test(test_create_printed_order),
    };
    return cmocka_run_group_tests(tests, NULL, NULL);
}
