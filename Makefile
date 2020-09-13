CC = g++

CFLAGS = -std=c++11

SRC = src
OBJ = obj

GRAPH = graph
GRAPH_OBJ = obj/graph.o obj/utility.o
EX_GRAPH = ex_graph
EX_GRAPH_OBJ = obj/ex_graph.o obj/utility.o
EX_GRAPH_OS = ex_graph_os
EX_GRAPH_OS_OBJ = obj/ex_graph_os.o obj/utility.o

all: $(GRAPH)

$(GRAPH): $(GRAPH_OBJ)
	$(CC) $(CFLAGS) $(GRAPH_OBJ) -o $(GRAPH)

$(EX_GRAPH): $(EX_GRAPH_OBJ)
	$(CC) $(CFLAGS) $(EX_GRAPH_OBJ) -o $(EX_GRAPH)

$(EX_GRAPH_OS): $(EX_GRAPH_OS_OBJ)
	$(CC) $(CFLAGS) $(EX_GRAPH_OS_OBJ) -o $(EX_GRAPH_OS)

obj/utility.o: src/utility.cpp
	$(CC) $(CFLAGS) -c $< -o $@

obj/graph.o: src/graph.cpp
	$(CC) $(CFLAGS) -c $< -o $@

obj/ex_graph.o: src/ex_graph.cpp
	$(CC) $(CFLAGS) -c $< -o $@

obj/ex_graph_os.o: src/ex_graph_os.cpp
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf obj/*.o $(GRAPH) $(EX_GRAPH) $(EX_GRAPH_OS)
