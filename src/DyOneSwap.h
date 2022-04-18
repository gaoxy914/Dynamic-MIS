#ifndef __DY_ONE_SWAP__
#define __DY_ONE_SWAP__

#include <string>
#include <vector>
#include <cassert>
#include <iostream>
#include <fstream>
#include <cstdio>
#include <algorithm>
#include <cmath>
#include <queue>
#include <unordered_map>
#include <ios>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <fcntl.h>
#include <stdio.h>
#include <inttypes.h>
#include <semaphore.h>
#include <string.h>
#include <stdlib.h>
#include <set>

#define _LINUX_

#ifdef _LINUX_
    #include <sys/time.h>
#endif

#define NDEBUG

#define _GRAPH_PATH "Graph/"
#define _MIS_PATH "MIS/"
#define _INST_PATH "Inst/"

// #define _PERTURBATION_

using namespace std;

struct Update {
    int type, u, v;
};

struct Node {
    int id, status, degree, count;
    vector<int> edges;
    unordered_map<int, bool> L;
    Node() : id(0), status(0), degree(0), count(0) {}
    void _insert_edge(const int& id);
    void _delete_edge(const int& id);
    void _insert_L(const int& id);
    void _delete_L(const int& id);
};

class Graph {
public:
    int n, mis;
    long m;
    long long utime;
    vector<Node> nodes;
    unordered_map<int, vector<int> > candidates;

    void _insert_candidate(const int& u, const int& v);
    void _delete_vertex(const int& id);
    void _insert_edge(int u, int v);
    void _delete_edge(int u, int v);
    void _move_in(const int& id);
    void _move_out(const int& id);
    void _swap(const int& u, const int& v, const int& w);
    void _extend_oneswap();

    Graph();
    ~Graph();
    void load_graph(const char* path);
    void load_mis(const char* path);
    void eff_exp(const char* path, const int& num);
    void check();
};

#endif