TARGET=sim
CXX=g++
LIBRARIES=-L../../lib/ -L../../restbed/distribution/library/
INCLUDES=-I../../src/ -I../../restbed/distribution/include/
GDRIFTXX=../../lib/libgdrift++.a
RESTBED=../../../restbed/distribution/library/librestbed.a
CXXFLAGS=-std=c++14 -Wall -g -ggdb -O3 -pthread -lboost_thread -lboost_system -larmadillo -fopenmp

OBJS=obj/sim.o

$(TARGET):$(OBJS)
			 $(CXX) $(LIBRARIES) $(INCLUDES) $^ -o $@ $(CXXFLAGS) $(GDRIFTXX) $(RESTBED) 

obj/sim.o:src/sim.cc
			      $(CXX) -c $(LIBRARIES) $(INCLUDES) $^ -o $@ $(CXXFLAGS)

clean:
		${RM} $(TARGET) $(OBJS)
