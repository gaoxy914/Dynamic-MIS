CC = g++

CFLAGS = -std=c++11

GRAPHTEST = graph
OBJGRAPH = graph.o

all: $(GRAPHTEST)

graph: $(OBJGRAPH)
	$(CC) $(CFLAGS) -O3 $(OBJGRAPH) -o $(GRAPHTEST)

graph.o: graph.cpp
	$(CC) $(CFLAGS) -O3 -c $< -o $@

clean:
	rm *.o $(GRAPHTEST)


