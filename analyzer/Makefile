TARGET=../build/bin/gdrift_service_analyzer
CXX=g++
LIBRARIES=-L/usr/local/library
INCLUDES=-I/usr/local/include/bsoncxx/v_noabi/ -I/usr/local/include/mongocxx/v_noabi/ -I/usr/local/include -I./include -I../include -I/usr/local/include/gdrift++
CXXFLAGS=-std=c++11 -Wall -g -ggdb -O3 -pthread -lboost_thread -lboost_system -larmadillo -lcurl -fopenmp -lgdrift++ -lrestbed -lssl -lcrypto -ldl -lmongocxx -lbsoncxx

OBJS=obj/analyzer.o obj/Analyzer.o obj/FileParser.o obj/MultipartFormParser.o obj/CuckooEgg.o obj/CuckooNest.o obj/CuckooSearch.o obj/Cuckoo.o

$(TARGET):$(OBJS)
			 $(CXX) $(LIBRARIES) $(INCLUDES) $^ -o $@ $(CXXFLAGS)

obj/analyzer.o:src/analyzer.cc
			      $(CXX) -c $(LIBRARIES) $(INCLUDES) $^ -o $@ $(CXXFLAGS)
obj/Analyzer.o:src/Analyzer.cc include/Analyzer.h
					 $(CXX) -c $< -o $@ $(CXXFLAGS) $(LIBRARIES) $(INCLUDES)
obj/FileParser.o:src/FileParser.cc include/FileParser.h
					 $(CXX) -c $< -o $@ $(CXXFLAGS) $(LIBRARIES) $(INCLUDES)
obj/MultipartFormParser.o:src/MultipartFormParser.cc include/MultipartFormParser.h
					 $(CXX) -c $< -o $@ $(CXXFLAGS) $(LIBRARIES) $(INCLUDES)
obj/CuckooEgg.o:src/CuckooEgg.cc include/CuckooEgg.h
					 $(CXX) -c $< -o $@ $(CXXFLAGS) $(LIBRARIES) $(INCLUDES)
obj/CuckooNest.o:src/CuckooNest.cc include/CuckooNest.h
					 $(CXX) -c $< -o $@ $(CXXFLAGS) $(LIBRARIES) $(INCLUDES)
obj/CuckooSearch.o:src/CuckooSearch.cc include/CuckooSearch.h
					 $(CXX) -c $< -o $@ $(CXXFLAGS) $(LIBRARIES) $(INCLUDES)
obj/Cuckoo.o:src/Cuckoo.cc include/Cuckoo.h
					 $(CXX) -c $< -o $@ $(CXXFLAGS) $(LIBRARIES) $(INCLUDES)

clean:
		${RM} $(TARGET) $(OBJS)
