TARGET=controller
CXX=g++-4.9
LIBRARIES=-L../libgdrift/lib/ 
INCLUDES=-I../libgdrift/src/ 
CXXFLAGS=-std=c++11 -Wall -g -ggdb -O3 -pthread -lboost_thread -lboost_system -larmadillo -lcurl -fopenmp -lgdrift++ -lrestbed -lssl -lcrypto -ldl

OBJS=obj/controller.o obj/Semaphore.o obj/Controller.o

$(TARGET):$(OBJS)
			 $(CXX) $(LIBRARIES) $(INCLUDES) $^ -o $@ $(CXXFLAGS)

obj/controller.o:src/controller.cc
			      $(CXX) -c $(LIBRARIES) $(INCLUDES) $^ -o $@ $(CXXFLAGS)
obj/Semaphore.o:../util/Semaphore.cc ../util/Semaphore.h
					 $(CXX) -c $< -o $@ $(CXXFLAGS) 
obj/Controller.o:src/Controller.cc src/Controller.h
					  $(CXX) -c $< -o $@ $(CXXFLAGS) $(LIBRARIES) $(INCLUDES)

clean:
		${RM} $(TARGET) $(OBJS)
