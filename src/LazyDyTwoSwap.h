#ifndef __DY_TWO_SWAP__
#define __DY_TWO_SWAP__

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
#include <map>

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
    int id, status, count, is1, is2;
    vector<int> edges;
    Node() : id(0), status(0), count(0), is1(-1), is2(-1) {}
    bool _exist_edge(const int &id);
    void _insert_edge(const int& id);
    void _delete_edge(const int& id);
    pair<int, int> _get_pair();
};

struct hash_pair {
    template <class T1, class T2>
    size_t operator() (const pair<T1, T2>& p) const {
        auto hash1 = hash<T1>{}(p.first);
        auto hash2 = hash<T2>{}(p.second);
        return hash1^hash2;
    }
};

class Graph {
public:
    int n, mis;
    long m;
    long long utime;
    vector<Node> nodes;
    unordered_map<int, vector<int> > candidates1;
    map<pair<int, int>, vector<int> > candidates2;

    void _insert_candidate1(const int& u, const int& v);
    void _insert_candidate2(const pair<int, int>& p, const int& v);
    void _find_candidate(const int& id);
    void _delete_vertex(const int& id);
    void _insert_edge(int u, int v);
    void _delete_edge(int u, int v);
    void _move_in(const int& id);
    int _move_out(const int& id);
    void _extend();
    void _oneswap();
    void _twoswap();
    bool _common_neighbors(const int& u, const int& v);

    Graph();
    ~Graph();
    void load_graph(const char* path);
    void load_mis(const char* path);
    void eff_exp(const char* path, const int& num);
    void check();
};

#endif