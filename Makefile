CC = g++

CFLAGS = -std=c++11

GRAPHTEST = graph
OBJGRAPH = graph.o

all: $(GRAPHTEST)

graph: $(OBJGRAPH)
	$(CC) $(CFLAGS) $(OBJGRAPH) -o $(GRAPHTEST)

graph.o: graph.cpp
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm *.o $(GRAPHTEST)


