TARGET=scheduler
CXX=g++-4.9
LIBRARIES=-L../gdriftlib/lib/ -L/usr/local/lib
INCLUDES=-I../gdriftlib/src/ -I/usr/local/include/bsoncxx/v_noabi/ -I/usr/local/include/mongocxx/v_noabi/ -I/usr/local/include
CXXFLAGS=-std=c++11 -Wall -g -ggdb -O3 -pthread -lboost_thread -lboost_system -larmadillo -lcurl -fopenmp -lgdrift++ -lrestbed -lssl -lcrypto -ldl -lmongocxx -lbsoncxx

OBJS=obj/scheduler.o obj/Scheduler.o obj/Semaphore.o

$(TARGET):$(OBJS)
			 $(CXX) $(LIBRARIES) $(INCLUDES) $^ -o $@ $(CXXFLAGS)

obj/scheduler.o:src/scheduler.cc
			      $(CXX) -c $(LIBRARIES) $(INCLUDES) $^ -o $@ $(CXXFLAGS)
obj/Scheduler.o:src/Scheduler.cc src/Scheduler.h
					 $(CXX) -c $< -o $@ $(CXXFLAGS) $(LIBRARIES) $(INCLUDES)
obj/Semaphore.o:../util/Semaphore.cc ../util/Semaphore.h
					 $(CXX) -c $< -o $@ $(CXXFLAGS) $(LIBRARIES) $(INCLUDES)

clean:
		${RM} $(TARGET) $(OBJS)
