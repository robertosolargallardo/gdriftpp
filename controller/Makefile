TARGET=sim
CXX=g++-4.9
LIBRARIES=-L../gdriftlib/lib/ 
INCLUDES=-I../gdriftlib/src/ 
CXXFLAGS=-std=c++11 -Wall -g -ggdb -O3 -pthread -lboost_thread -lboost_system -larmadillo -lcurl -fopenmp -lgdrift++ -lrestbed -lssl -lcrypto -ldl

OBJS=obj/sim.o obj/Semaphore.o

$(TARGET):$(OBJS)
			 $(CXX) $(LIBRARIES) $(INCLUDES) $^ -o $@ $(CXXFLAGS)

obj/sim.o:src/sim.cc
			      $(CXX) -c $(LIBRARIES) $(INCLUDES) $^ -o $@ $(CXXFLAGS)
obj/Semaphore.o:src/Semaphore.cc src/Semaphore.h
					 $(CXX) -c $< -o $@ $(CXXFLAGS) 

clean:
		${RM} $(TARGET) $(OBJS)
