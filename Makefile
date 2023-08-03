CC       = gcc
CFLAGS   = -Wall -Werror -Wvla -O0 -std=c11 -g -fsanitize=address,leak
SRC      = pe_exchange.c pe_trader.c pe_orderbook.c pe_printing.c pe_pipes.c pe_setup.c
LDFLAGS  = -lm
BINARIES = pe_exchange pe_trader
OBJ      = $(SRC:.c=.o)

all: $(BINARIES)

.PHONY: clean
clean:
	rm -f *.o *.obj $(BINARIES)
	rm -f /tmp/pe_exchange_0
	rm -f /tmp/pe_trader_0

pe_exchange.o: pe_exchange.c pe_exchange.h
	$(CC) $(CFLAGS) -c $<

pe_orderbook.o: pe_orderbook.c pe_orderbook.h
	$(CC) $(CFLAGS) -c $<

pe_printing.o: pe_printing.c pe_printing.h
	$(CC) $(CFLAGS) -c $<

pe_pipes.o: pe_pipes.c pe_pipes.h
	$(CC) $(CFLAGS) -c $<

pe_setup.o: pe_setup.c pe_setup.h
	$(CC) $(CFLAGS) -c $<

pe_exchange: pe_exchange.o pe_orderbook.o pe_printing.o pe_pipes.o pe_setup.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

pe_trader: pe_trader.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

test:
	bash test_script.sh

run_tests:
	bash test_script.sh