GCC_FLAGS = -g -Wall -std=c++20 -pthread
INCL = -I include

bin/util.o: include/util/spinlock.h include/util/job_queue.h src/util/spinlock.cpp src/util/job_queue.cpp
	g++-10 -c src/util/spinlock.cpp $(GCC_FLAGS) $(INCL) -o bin/spinlock.o
	g++-10 -c src/util/job_queue.cpp $(GCC_FLAGS) $(INCL) -o bin/job_queue.o
	ld -r -o bin/util.o bin/spinlock.o bin/job_queue.o

bin/polling.o: src/polling/timing_poll_source.cpp include/polling/timing_poll_source.h include/polling/poll_source.h
	g++-10 -c src/polling/timing_poll_source.cpp $(GCC_FLAGS) $(INCL) -o bin/polling.o

bin/scheduler.o: src/scheduler.cpp include/scheduler.h bin/util.o
	g++-10 -c src/scheduler.cpp $(GCC_FLAGS) $(INCL) -o bin/scheduler.o 

build: bin/scheduler.o bin/util.o bin/polling.o
	g++-10 tests/main.cpp bin/scheduler.o bin/util.o bin/polling.o $(GCC_FLAGS) $(INCL) -o bin/main


stress:
	g++-10 tests/stress.cpp -g -Wall -I include -std=c++20 -o stress -pthread


queue: bin/util.o
	g++-10 tests/queue_test.cpp bin/util.o $(GCC_FLAGS) $(INCL) -o bin/queue