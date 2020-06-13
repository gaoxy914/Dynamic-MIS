#ifndef _GRAPH_H
#define _GRAPH_H

#include <string>
#include <vector>
#include <cassert>
#include <cstdio>
#include <iostream>
#include <fstream>
#include <algorithm>

#define swap(u, v) {unsigned int temp = u; u = v; v = temp;}

#define _VERTEX_ADDITION 0
#define _VERTEX_DELETION 1
#define _EDGE_ADDITION 2
#define _EDGE_DELETION 3
#define _DEFAULT -1

#define _INST_NUM 1000

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
    _MIS, _DOMINATED, _UNVISITD, _DELETED
};

struct edge {
    unsigned int node_id;
    edge* next_edge;

    edge(unsigned int _node_id) : node_id(_node_id), next_edge(NULL) {}
};

struct node {
    unsigned int node_id;
    unsigned int degree;
    status node_status;
    // unsigned int offset;
    unsigned int counter;
    edge* edges;

    node() : node_id(0), degree(0), node_status(_UNVISITD), counter(0), edges(NULL) {}

    node(unsigned int _node_id, unsigned int degree = 0, status _node_status = _UNVISITD, unsigned int _counter = 0)
        : node_id(_node_id), degree(0), node_status(_node_status), counter(_counter), edges(NULL) {}
};

class graph {
private:
    string file_path;
    unsigned int n, m, mis;

    node* nodes;
    // unsigned int* edges;

public:
    graph(const char* _file_path);
    ~graph();

    void read_graph();

    void handle_update(const update& _update);

    void greedy();
    void greedy_dynamic();

    void show();
    void experiment(const char* _inst_file);

private:
    void check_mis();

    void add_vertex(const unsigned int& u);
    void delete_vertex(const unsigned int& u);
    void add_edge(unsigned int u, unsigned int v);
    void delete_edge(unsigned int u, unsigned int v);

    void update_neighbors(const unsigned int& u);
    void update_neighbors(const vector<unsigned int>& v_in, const vector<unsigned int>& v_out);

    void localserach_one_imp(const unsigned int& u, vector<unsigned int>& v_in, vector<unsigned int>& v_out);
    void localsearch_two_imp(const unsigned int& u, vector<unsigned int>& v_in, vector<unsigned int>& v_out);
};

#endif
