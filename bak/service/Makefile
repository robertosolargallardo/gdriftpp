TARGET=test
CXX=g++-4.9
LIBRARIES=-L/usr/local/lib
INCLUDES=-I/usr/local/include/bsoncxx/v_noabi/ -I/usr/local/include/mongocxx/v_noabi/ -I/usr/local/include
CXXFLAGS=-std=c++11 -Wall -g -ggdb -O3 -pthread -lboost_thread -lboost_system -larmadillo -lcurl -fopenmp -lrestbed -lssl -lcrypto -ldl -lmongocxx -lbsoncxx

OBJS=obj/test.o 

$(TARGET):$(OBJS)
			 $(CXX) $^ -o $@ $(CXXFLAGS) $(LIBRARIES) $(INCLUDES)

obj/test.o:src/test.cc
			      $(CXX) -c $^ -o $@ $(CXXFLAGS) $(LIBRARIES) $(INCLUDES)
clean:
		${RM} $(TARGET) $(OBJS)
