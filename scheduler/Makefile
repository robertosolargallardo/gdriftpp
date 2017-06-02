TARGET=scheduler
CXX=g++-4.9
LIBRARIES=-L/usr/local/lib
INCLUDES=-I/usr/local/include/bsoncxx/v_noabi/ -I/usr/local/include/mongocxx/v_noabi/ -I/usr/local/include -I./include -I../include -I/usr/local/include/gdrift++
CXXFLAGS=-std=c++11 -Wall -g -ggdb -O3 -pthread -lboost_thread -lboost_system -larmadillo -lcurl -fopenmp -lgdrift++ -lrestbed -lssl -lcrypto -ldl -lmongocxx -lbsoncxx

OBJS=obj/scheduler.o obj/Scheduler.o obj/SimulationSettings.o obj/Semaphore.o

$(TARGET):$(OBJS)
			 $(CXX) $(LIBRARIES) $(INCLUDES) $^ -o $@ $(CXXFLAGS)

obj/scheduler.o:src/scheduler.cc
			      $(CXX) -c $(LIBRARIES) $(INCLUDES) $^ -o $@ $(CXXFLAGS)
obj/Scheduler.o:src/Scheduler.cc include/Scheduler.h
					 $(CXX) -c $< -o $@ $(CXXFLAGS) $(LIBRARIES) $(INCLUDES)
obj/SimulationSettings.o:src/SimulationSettings.cc include/SimulationSettings.h
					 $(CXX) -c $< -o $@ $(CXXFLAGS) $(LIBRARIES) $(INCLUDES)
obj/Semaphore.o:../include/Semaphore.cc ../include/Semaphore.h
					 $(CXX) -c $< -o $@ $(CXXFLAGS) $(LIBRARIES) $(INCLUDES)

clean:
		${RM} $(TARGET) $(OBJS)
