GCC_FLAGS = -g -Wall -std=c++20 -pthread
INCL = -I include

bin/util.o: src/util/spinlock.cpp include/util/spinlock.h
	g++-10 -c src/util/spinlock.cpp $(GCC_FLAGS) $(INCL) -o bin/util.o

bin/polling.o: src/polling/timing_poll_source.cpp include/polling/timing_poll_source.h include/polling/poll_source.h
	g++-10 -c src/polling/timing_poll_source.cpp $(GCC_FLAGS) $(INCL) -o bin/polling.o

bin/scheduler.o: src/scheduler.cpp include/scheduler.h bin/util.o
	g++-10 -c src/scheduler.cpp $(GCC_FLAGS) $(INCL) -o bin/scheduler.o 

build: bin/scheduler.o bin/util.o bin/polling.o
	g++-10 tests/main.cpp bin/scheduler.o bin/util.o bin/polling.o $(GCC_FLAGS) $(INCL) -o bin/main


stress:
	g++-10 stress.cpp -g -Wall -I include -std=c++20 -o stress -pthread

