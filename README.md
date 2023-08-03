1. Describe how your exchange works.

The exchange relies on SIGUSR1, SIGCHLD and EPOLLHUP. There are two global signal queues for signals. These are circular arrays, and they are used in case the exchange receives many signals in quick succession, such that it receives a signal before it finishes processing the previous one. The epoll is used just in case a trader closes a pipe (meaning it has exited the exchange), but doesn't shut down entirely. 

Within my main, there is a while loop running. Within that, we first pause. Then, a loop runs until there is nothing in the SIGUSR1 queue. Once we have no SIGUSR1's left, we then check if there is anything in the SIGCHLD queue. Finally, we run an epoll_wait until either an EPOLLHUP or a signal is received. This happens until there are no active traders left. If a particular trader sends a SIGCHLD or closes their pipe, that trader's active status gets set to 0. We also keep a count of the number of active traders. 


2. Describe your design decisions for the trader and how it's fault-tolerant.

The auto trader places orders in response to messages it receives from the market. The market is then required to send an "ACCEPTED" message back to the auto trader. However, the market is often busy and may miss the signals that the auto trader sends after it has finished writing its order to the pipe. Therefore, every time the auto trader is done writin an order to the pipe and sending a SIGUSR1 signal, it will wait 0.3 seconds. If by the time the 0.3 seconds is up, no signal has been received, it will send another signal. It will keep doing this until it has received a signal and "ACCEPTED" message from the exchange.

The auto trader has a simple signal handler which simply sets a signal_received variable to 1.


3. Describe your tests and how to run them.

I have written a few unit tests for smaller functions that require less setup. These are contained in the tests/unit-tests.c file. 

Additionally, I have written various E2E tests, each contained within their own folder within tests/E2E. These tests each have at least one trader that reads from a "trader.in" file, and sends these hardcoded messages to the exchange. These traders will wait until a signal is received back from the exchange, then send the next message. The expected exchange output of these tests are contained in tests/E2E/random_test/exchange.out.

These tests can be run by calling: 
$ make
$ make test

This will run the bash file I have written, "test_script.sh".

This script runs the unit tests, then runs diff commands for each of the E2E tests. 
