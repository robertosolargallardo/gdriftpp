TARGET=controller
CXX=g++-4.9
LIBRARIES=-L/usr/local/lib
INCLUDES=-I/usr/local/include/bsoncxx/v_noabi/ -I/usr/local/include/mongocxx/v_noabi/ -I/usr/local/include -I./include -I../include -I/usr/local/include/gdrift++
CXXFLAGS=-std=c++11 -Wall -g -ggdb -O3 -pthread -lboost_thread -lboost_system -larmadillo -lcurl -fopenmp -lgdrift++ -lrestbed -lssl -lcrypto -ldl -lmongocxx -lbsoncxx

OBJS=obj/controller.o obj/Controller.o

$(TARGET):$(OBJS)
			 $(CXX) $(LIBRARIES) $(INCLUDES) $^ -o $@ $(CXXFLAGS)

obj/controller.o:src/controller.cc
			      $(CXX) -c $(LIBRARIES) $(INCLUDES) $^ -o $@ $(CXXFLAGS)
obj/Controller.o:src/Controller.cc include/Controller.h
					 $(CXX) -c $< -o $@ $(CXXFLAGS) $(LIBRARIES) $(INCLUDES)

clean:
		${RM} $(TARGET) $(OBJS)
