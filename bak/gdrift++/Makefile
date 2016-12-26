CFLAGS=-std=c++11 -g -O3 -Wall -ggdb -fopenmp -larmadillo
CC=g++-4.9
TARGET=lib/libgdrift++.a
OBJS=obj/Individual.o obj/Event.o obj/Population.o obj/Chromosome.o obj/EventList.o obj/Gene.o obj/Simulator.o obj/Pool.o

$(TARGET):$(OBJS)
			 ar rvs $(TARGET) $^

obj/Event.o:src/Event.cc src/Event.h
			 	  $(CC) $(CFLAGS) $< -c -o $@
obj/Individual.o:src/Individual.cc src/Individual.h
			 	  $(CC) $(CFLAGS) $< -c -o $@
obj/EventList.o:src/EventList.cc src/EventList.h
			 	  $(CC) $(CFLAGS) $< -c -o $@
obj/Population.o:src/Population.cc src/Population.h src/Individual.h src/Event.cc src/Event.h
			 		  $(CC) $(CFLAGS) $< -c -o $@
obj/Chromosome.o:src/Chromosome.cc src/Chromosome.h
			 		  $(CC) $(CFLAGS) $< -c -o $@
obj/Gene.o:src/Gene.cc src/Gene.h
			 $(CC) $(CFLAGS) $< -c -o $@
obj/Simulator.o:src/Simulator.cc src/Simulator.h
			 $(CC) $(CFLAGS) $< -c -o $@

obj/Pool.o:src/Pool.cc src/Pool.h
			 $(CC) $(CFLAGS) $< -c -o $@

clean:
		${RM} $(TARGET) $(OBJS) *~
