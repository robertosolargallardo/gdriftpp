TARGET=results
CXX=g++-4.9
LIBRARIES=-L/usr/local/lib -L../libgdrift/lib/ 
INCLUDES=-I/usr/local/include/bsoncxx/v_noabi/ -I/usr/local/include/mongocxx/v_noabi/ -I/usr/local/include  -I../libgdrift/src
CXXFLAGS=-std=c++11 -Wall -g -ggdb -O3 -pthread -lboost_thread -lboost_system -larmadillo -lcurl -fopenmp -lrestbed -lssl -lcrypto -ldl -lmongocxx -lbsoncxx -lgdrift++

OBJS=obj/results.o obj/Results.o obj/Semaphore.o

$(TARGET):$(OBJS)
			 $(CXX) $^ -o $@ $(CXXFLAGS) $(LIBRARIES) $(INCLUDES)

obj/results.o:src/results.cc
			      $(CXX) -c $^ -o $@ $(CXXFLAGS) $(LIBRARIES) $(INCLUDES)
obj/Results.o:src/Results.cc src/Results.h
			      $(CXX) -c $< -o $@ $(CXXFLAGS) $(LIBRARIES) $(INCLUDES)
obj/Semaphore.o:../util/Semaphore.cc ../util/Semaphore.h
					 $(CXX) -c $< -o $@ $(CXXFLAGS) $(LIBRARIES) $(INCLUDES)
clean:
		${RM} $(TARGET) $(OBJS)
