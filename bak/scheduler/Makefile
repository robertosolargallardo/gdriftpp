TARGET=scheduler
CXX=g++-4.9
#LIBRARIES=-L../gdrift++/lib/ 
#INCLUDES=-I../gdrift++/src/ 
#CXXFLAGS=-std=c++11 -Wall -g -ggdb -O3 -pthread -lboost_thread -lboost_system -larmadillo -lcurl -fopenmp -lgdrift++ -lrestbed -lssl -lcrypto -ldl
LIBRARIES=-L/usr/local/lib -L../gdrift++/lib/ 
INCLUDES=-I/usr/local/include/bsoncxx/v_noabi/ -I/usr/local/include/mongocxx/v_noabi/ -I/usr/local/include -I../gdrift++/src/ 
CXXFLAGS=-std=c++11 -Wall -g -ggdb -O3 -pthread -lboost_thread -lboost_system -larmadillo -lcurl -fopenmp -lrestbed -lssl -lcrypto -ldl -lmongocxx -lbsoncxx 

OBJS=obj/scheduler.o obj/ConfigGenerator.o

$(TARGET):$(OBJS)
			 $(CXX) $(LIBRARIES) $(INCLUDES) $^ -o $@ $(CXXFLAGS)

obj/scheduler.o:src/scheduler.cc
			      $(CXX) -c $(LIBRARIES) $(INCLUDES) $^ -o $@ $(CXXFLAGS)
obj/ConfigGenerator.o:src/ConfigGenerator.cc src/ConfigGenerator.h
			$(CXX) -c $< -o $@ $(CXXFLAGS)

clean:
		${RM} $(TARGET) $(OBJS)
