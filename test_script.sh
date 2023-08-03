gcc ./tests/unit-tests.c pe_setup.c pe_orderbook.c pe_printing.c pe_pipes.c libcmocka-static.a -lm -o pe_exchange_test 
./pe_exchange_test

rm -f /tmp/pe_exchange_0
rm -f /tmp/pe_trader_0


gcc -o ./tests/E2E/amend_orders/trader ./tests/E2E/amend_orders/trader.c
gcc pe_exchange.c pe_orderbook.c pe_printing.c pe_pipes.c pe_setup.c -lm -o pe_exchange
diff --strip-trailing-cr ./tests/E2E/amend_orders/exchange.out <(./pe_exchange < ./pe_exchange ./tests/E2E/valid_products.txt ./tests/E2E/amend_orders/trader)

rm -f /tmp/pe_exchange_0
rm -f /tmp/pe_trader_0

gcc -o ./tests/E2E/buy_levels/trader ./tests/E2E/buy_levels/trader.c
gcc pe_exchange.c pe_orderbook.c pe_printing.c pe_pipes.c pe_setup.c -lm -o pe_exchange
diff --strip-trailing-cr ./tests/E2E/buy_levels/exchange.out <(./pe_exchange < ./pe_exchange ./tests/E2E/valid_products.txt ./tests/E2E/buy_levels/trader)


rm -f /tmp/pe_exchange_0
rm -f /tmp/pe_trader_0

gcc -o ./tests/E2E/sell_levels/trader ./tests/E2E/sell_levels/trader.c
gcc pe_exchange.c pe_orderbook.c pe_printing.c pe_pipes.c pe_setup.c -lm -o pe_exchange
diff --strip-trailing-cr ./tests/E2E/sell_levels/exchange.out <(./pe_exchange < ./pe_exchange ./tests/E2E/valid_products.txt ./tests/E2E/sell_levels/trader)


rm -f /tmp/pe_exchange_0
rm -f /tmp/pe_trader_0

gcc -o ./tests/E2E/cancel_orders/trader ./tests/E2E/cancel_orders/trader.c
gcc pe_exchange.c pe_orderbook.c pe_printing.c pe_pipes.c pe_setup.c -lm -o pe_exchange
diff --strip-trailing-cr ./tests/E2E/cancel_orders/exchange.out <(./pe_exchange < ./pe_exchange ./tests/E2E/valid_products.txt ./tests/E2E/cancel_orders/trader)


rm -f /tmp/pe_exchange_0
rm -f /tmp/pe_trader_0

gcc -o ./tests/E2E/invalid_orders/trader ./tests/E2E/invalid_orders/trader.c
gcc pe_exchange.c pe_orderbook.c pe_printing.c pe_pipes.c pe_setup.c -lm -o pe_exchange
diff --strip-trailing-cr ./tests/E2E/invalid_orders/exchange.out <(./pe_exchange < ./pe_exchange ./tests/E2E/valid_products.txt ./tests/E2E/invalid_orders/trader)


rm -f /tmp/pe_exchange_0
rm -f /tmp/pe_trader_0

gcc -o ./tests/E2E/invalid_prod_name/trader ./tests/E2E/invalid_prod_name/trader.c
gcc pe_exchange.c pe_orderbook.c pe_printing.c pe_pipes.c pe_setup.c -lm -o pe_exchange
diff --strip-trailing-cr ./tests/E2E/invalid_prod_name/exchange.out <(./pe_exchange < ./pe_exchange ./tests/E2E/invalid_prod_name/products.txt ./tests/E2E/invalid_prod_name/trader)


rm -f /tmp/pe_exchange_0
rm -f /tmp/pe_trader_0

gcc -o ./tests/E2E/invalid_prod_num/trader ./tests/E2E/invalid_prod_num/trader.c
gcc pe_exchange.c pe_orderbook.c pe_printing.c pe_pipes.c pe_setup.c -lm -o pe_exchange
diff --strip-trailing-cr ./tests/E2E/invalid_prod_num/exchange.out <(./pe_exchange < ./pe_exchange ./tests/E2E/invalid_prod_num/products.txt ./tests/E2E/invalid_prod_num/trader)


rm -f /tmp/pe_exchange_0
rm -f /tmp/pe_trader_0

gcc -o ./tests/E2E/prod_too_long/trader ./tests/E2E/prod_too_long/trader.c
gcc pe_exchange.c pe_orderbook.c pe_printing.c pe_pipes.c pe_setup.c -lm -o pe_exchange
diff --strip-trailing-cr ./tests/E2E/prod_too_long/exchange.out <(./pe_exchange < ./pe_exchange ./tests/E2E/prod_too_long/products.txt ./tests/E2E/prod_too_long/trader)


rm -f /tmp/pe_exchange_0
rm -f /tmp/pe_trader_0

gcc -o ./tests/E2E/too_many_prods/trader ./tests/E2E/too_many_prods/trader.c
gcc pe_exchange.c pe_orderbook.c pe_printing.c pe_pipes.c pe_setup.c -lm -o pe_exchange
diff --strip-trailing-cr ./tests/E2E/too_many_prods/exchange.out <(./pe_exchange < ./pe_exchange ./tests/E2E/too_many_prods/products.txt ./tests/E2E/too_many_prods/trader)


rm -f /tmp/pe_exchange_0
rm -f /tmp/pe_trader_0

gcc -o ./tests/E2E/too_few_prods/trader ./tests/E2E/too_few_prods/trader.c
gcc pe_exchange.c pe_orderbook.c pe_printing.c pe_pipes.c pe_setup.c -lm -o pe_exchange
diff --strip-trailing-cr ./tests/E2E/too_few_prods/exchange.out <(./pe_exchange < ./pe_exchange ./tests/E2E/too_few_prods/products.txt ./tests/E2E/too_few_prods/trader)


rm -f /tmp/pe_exchange_0
rm -f /tmp/pe_trader_0

gcc -o ./tests/E2E/match_with_self/trader ./tests/E2E/match_with_self/trader.c
gcc pe_exchange.c pe_orderbook.c pe_printing.c pe_pipes.c pe_setup.c -lm -o pe_exchange
diff --strip-trailing-cr ./tests/E2E/match_with_self/exchange.out <(./pe_exchange < ./pe_exchange ./tests/E2E/valid_products.txt ./tests/E2E/match_with_self/trader)


rm -f /tmp/pe_exchange_0
rm -f /tmp/pe_trader_0
rm -f /tmp/pe_exchange_1
rm -f /tmp/pe_trader_1

gcc -o ./tests/E2E/match_orders/trader1 ./tests/E2E/match_orders/trader1.c
gcc -o ./tests/E2E/match_orders/trader2 ./tests/E2E/match_orders/trader2.c
gcc pe_exchange.c pe_orderbook.c pe_printing.c pe_pipes.c pe_setup.c -lm -o pe_exchange
diff --strip-trailing-cr ./tests/E2E/match_orders/exchange.out <(./pe_exchange < ./pe_exchange ./tests/E2E/valid_products.txt ./tests/E2E/match_orders/trader1 ./tests/E2E/match_orders/trader2)


rm -f /tmp/pe_exchange_0
rm -f /tmp/pe_trader_0
rm -f /tmp/pe_exchange_1
rm -f /tmp/pe_trader_1

gcc -o ./tests/E2E/close_pipe/trader ./tests/E2E/close_pipe/trader.c
gcc pe_exchange.c pe_orderbook.c pe_printing.c pe_pipes.c pe_setup.c -lm -o pe_exchange
diff --strip-trailing-cr ./tests/E2E/close_pipe/exchange.out <(./pe_exchange < ./pe_exchange ./tests/E2E/valid_products.txt ./tests/E2E/close_pipe/trader)