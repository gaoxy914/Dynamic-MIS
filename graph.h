#ifndef _GRAPH_H
#define _GRAPH_H

#include "misheader.h"

#define _EDGE_ADDITION 0
#define _EDGE_DELETION 1
#define _VERTEX_ADDITION 2
#define _VERTEX_DELETION 3
#define _DEFAULT -1

#define _INST_NUM 1000

#define GRAPH_PATH "Graphdata/"
#define MIS_PATH "InitialMis/"
#define INST_PATH "Instruction/"

using namespace std;

// FILE* open_file(const char* file_path, const char* mode);

struct update {
    int type;
    unsigned int u;
    unsigned int v;
    update(int _type = -1, unsigned int _u = 0, unsigned int _v = 0)
        : type(_type), u(_u), v(_v) {}
};


enum status {
    _MIS, _NOMIS, _UNVISITED, _CONFLICT, _DELETED
};

struct edge {
    unsigned int node_id;
    edge* next_edge;

    edge(unsigned int _node_id) : node_id(_node_id), next_edge(NULL) {}
};

struct node {
    unsigned int node_id; // start from 1
    unsigned int degree;
    status node_status;
    unsigned int counter;
    edge* edges;

    node() : node_id(0), degree(0), node_status(_UNVISITED), counter(0), edges(NULL) {}

    node(unsigned int _node_id) : node_id(_node_id), degree(0), node_status(_UNVISITED),
        counter(0), edges(NULL) {}
};

class graph {
private:
    string file_path;
    string mis_path;
    // string inst_path;
    unsigned int n, m, mis;

    node* nodes;
    // unsigned int* edges;

    long long time;

public:
    graph(const char* _file_path, const char* _mis_path);
    // graph(const char* _file_path);
    graph(unsigned int _n);
    ~graph();

    void read_graph();
    void read_mis();

    void show();

    void greedy();
    void experiment(const char* _inst_file);

private:
    void handle_vertex_deletion(unsigned int u);
    void handle_edge_addition(unsigned int u, unsigned int v);
    void handle_edge_deletion(unsigned int u, unsigned int v);

    void check_mis();
    void check_swap();

    void add_into_IS(unsigned int u);
    void remove_from_IS(unsigned int u);
    void swap(const vector<unsigned int>& v_out, const vector<unsigned int>& v_in);

    void add_node(unsigned int index, unsigned int node_id);

    void add_vertex(const unsigned int& u);
    void delete_vertex(const unsigned int& u);
    void add_edge(unsigned int u, unsigned int v);
    void delete_edge(unsigned int u, unsigned int v);

    void greedy(vector<unsigned int>& I);
    void greedy_dynamic(vector<unsigned int>& I);

    int one_swapable_vertex(unsigned int u, vector<unsigned int>& I);
    int two_swapable_vertex(unsigned int u, unsigned int v, vector<unsigned int>& I);

};

#endif
