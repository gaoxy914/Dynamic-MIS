#ifndef _GRAPH_H
#define _GRAPH_H

#include "utility.h"

typedef unsigned int ui;

using namespace std;

// FILE* open_file(const char* file_path, const char* mode);

struct update_t {
    int type;
    ui u;
    ui v;
    update_t(int _type = -1, ui _u = 0, ui _v = 0)
        : type(_type), u(_u), v(_v) {}
};


enum status {
    _MIS, _NOMIS, _UNVISITED, _CONFLICT, _DELETED
};

struct edge_t {
    ui node_id;
    edge_t* next_edge;

    edge_t(ui _node_id) : node_id(_node_id), next_edge(NULL) {}
};

struct node_t {
    ui node_id; // start from 1
    ui degree;
    status node_status;
    ui counter;
    edge_t* edges;

    node_t() : node_id(0), degree(0), node_status(_UNVISITED), counter(0), edges(NULL) {}

    node_t(ui _node_id) : node_id(_node_id), degree(0), node_status(_UNVISITED),
        counter(0), edges(NULL) {}
};

class graph {
    string _file_path;
    string _mis_path;
    ui _n, _m, _mis;

    node_t* _nodes;
    long long _utime;

    void _simple(update_t update);
    void _handle_vertex_deletion(ui u);
    void _handle_edge_addition(ui u, ui v);
    void _handle_edge_deletion(ui u, ui v);

    void _check_mis();
    void _check_swap();

    void _add_into_IS(ui u);
    void _remove_from_IS(ui u);
    void _swap(const vector<ui>& v_out, const vector<ui>& v_in);

    void _add_node(ui index, ui node_id);

    void _add_vertex(const ui& u);
    void _delete_vertex(const ui& u);
    void _add_edge(ui u, ui v);
    void _delete_edge(ui u, ui v);

    void _greedy(vector<ui>& I);
    void _greedy_dynamic(vector<ui>& I);

    ui _one_swapable_vertex(ui u, vector<ui>& I);
    ui _two_swapable_vertex(ui u, ui& v, vector<ui>& I);
public:
    graph(const char* _file_path, const char* _mis_path);
    graph(ui n);
    ~graph();

    void read_graph();
    void read_mis();

    void show();

    void greedy();
    void experiment(const char* _inst_file);
    void experiment2(const char* _inst_file);
};

#endif
