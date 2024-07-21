GCC_FLAGS = -g -Wall -std=c++20 -pthread
INCL = -I include

bin/util.o: include/util/spinlock.h src/util/spinlock.cpp
	g++-10 -c src/util/spinlock.cpp $(GCC_FLAGS) $(INCL) -o bin/spinlock.o
	ld -r -o bin/util.o bin/spinlock.o

bin/polling.o: src/polling/timing_poll_source.cpp include/polling/timing_poll_source.h include/polling/poll_source.h
	g++-10 -c src/polling/timing_poll_source.cpp $(GCC_FLAGS) $(INCL) -o bin/polling.o

bin/scheduler.o: src/scheduler/scheduler.cpp include/scheduler/scheduler.h src/scheduler/circular_queue.cpp include/scheduler/circular_queue.h bin/util.o
	g++-10 -c src/scheduler/circular_queue.cpp $(GCC_FLAGS) $(INCL) -o bin/circular_queue.o
	g++-10 -c src/scheduler/scheduler.cpp $(GCC_FLAGS) $(INCL) -o bin/scheduler_.o 
	ld -r -o bin/scheduler.o bin/scheduler_.o bin/circular_queue.o

build: bin/scheduler.o bin/util.o bin/polling.o
	g++-10 -std=c++20 tests/io_test.cpp bin/scheduler.o bin/util.o bin/polling.o $(GCC_FLAGS) $(INCL) -o bin/main -lrt

stress:
	g++-10 tests/stress.cpp -g -Wall -I include -std=c++20 -o stress -pthread


queue: bin/scheduler.o bin/util.o
	g++-10 tests/queue_test.cpp bin/scheduler.o bin/util.o $(GCC_FLAGS) $(INCL) -o bin/queue