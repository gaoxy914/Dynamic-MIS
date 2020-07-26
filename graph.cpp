#include "graph.h"

/* FILE* open_file(const char* file_path, const char* mode){
    FILE* file = fopen(file_path, mode);
    if (file == NULL) {
        cout << "file: " << file_path << " open failed." << endl;
        exit(1);
    }
    return file;
} */

graph::graph(const char* _file_path, const char * _mis_path) {
// graph::graph(const char* _file_path) {
    file_path = string(_file_path);
    mis_path = string(_mis_path);
    n = m = mis = time = 0;
    nodes = NULL;
    // edges = NULL;
}

graph::graph(unsigned int _n) {
    file_path = mis_path = "";
    n = _n;
    m = mis = 0;
    nodes = NULL;
    nodes = new node[n + 1];
}

graph::~graph() {
    if (nodes != NULL) {
        delete[] nodes;
        nodes = NULL;
    }
    /* if (edges != null) {
        delete[] edges;
        edges = null;
    } */
}

/*
 * graph file format
 * n m
 * u v // m lines such that (u, v) in E
 */
void graph::read_graph() {
    // FILE* file = open_file(file_path.c_str(), "rb");
    ifstream infile((GRAPH_PATH + file_path).c_str());
    if (!infile.is_open()) {
        cout << "file: " << file_path << " open failed." << endl;
        exit(1);
    }

    infile >> n >> m;
    cout << "n = " << n << "; m = " << m << endl;

    if (nodes == NULL) nodes = new node[n + 1];
    for (int i = 1; i <= n; ++ i) {
        nodes[i] = node((unsigned int)i);
    }

    unsigned int u, v;
    for (int i = 0; i < m; ++ i) {
        infile >> u >> v;
        add_edge(u + 1, v + 1);
    }

#ifndef NDEBUG
    long long sum_degree = 0;
    for (int i = 1; i <= n; ++ i) {
        sum_degree += nodes[i].degree;
    }
    if (sum_degree != 2*m)
        cout << "wrong in input graph.\n";
#endif
    infile.close();
}

/*
 * mis file format
 * mis
 * u // u in IS
 */
void graph::read_mis() {
    ifstream infile((MIS_PATH + mis_path).c_str());
    if (!infile.is_open()) {
        cout << "file: " << mis_path << " open failed." << endl;
        exit(1);
    }
    infile >> mis;
    cout << "mis = " << mis << endl;
    unsigned int u;
    for (int i = 0; i < mis; ++i) {
        infile >> u;
        add_into_IS(u + 1);
    }
    infile.close();
}


void graph::show() {
    for (int i = 1; i <= n; ++i) {
        cout << nodes[i].node_id << " degree: " << nodes[i].degree
            << " node status: " << nodes[i].node_status
            << " counter: " << nodes[i].counter << " adjacent list : ";

        edge* p_edge = nodes[i].edges;
        while (p_edge != NULL) {
            cout << p_edge->node_id << ", ";
            p_edge = p_edge->next_edge;
        }
        cout << endl;
    }
}


void graph::greedy() {
    unsigned int* degree_buckets = new unsigned int[n];
    memset(degree_buckets, 0, sizeof(unsigned int)*n);
    for (int i = 1; i <= n; ++i)
        degree_buckets[nodes[i].degree]++;

    for (int i = 1; i < n; ++i)
        degree_buckets[i] += degree_buckets[i - 1];

    // order[i]  = j means that the order of node i is j
    unsigned int* order = new unsigned int[n + 1];
    for (int i = 1; i <= n; ++i) {
        order[i] = degree_buckets[nodes[i].degree];
        degree_buckets[nodes[i].degree]--;
    }

    // greedy_order[i] = j means that No.i is node j
    unsigned int* greedy_order = new unsigned int[n + 1];
    for (int i = 1; i <= n; ++i) {
        greedy_order[order[i]] = i;
    }

    for (int i = 1; i <= n; ++i) {
        if (nodes[greedy_order[i]].node_status == _UNVISITED) {
            mis ++;
            add_into_IS(greedy_order[i]);
        }
    }

    cout << "Greedy MIS: " << mis << endl;

    delete[] degree_buckets;
    delete[] order;
    delete[] greedy_order;

#ifndef NDEBUG
    check_mis();
#endif
}

void graph::experiment(const char* _inst_path) {
    string inst_path = string(_inst_path);
    vector<update> updates;
    ifstream infile((INST_PATH + inst_path).c_str());
    if (!infile.is_open()) {
        cout << "instruction file: " << _inst_path << " open failed." << endl;
        exit(1);
    }
    // check_swap();

    update _update;
    for (int i = 0; i < _INST_NUM; ++i) {
        infile >> _update.type;
        switch(_update.type) {
        case _VERTEX_ADDITION:
            infile >> _update.u;
            break;
        case _VERTEX_DELETION:
            infile >> _update.u;
            break;
        case _EDGE_ADDITION:
            infile >> _update.u >> _update.v;
            break;
        case _EDGE_DELETION:
            infile >> _update.u >> _update.v;
            break;
        default:
            break;
        }
        updates.push_back(_update);
    }
    infile.close();

    for (int i = 0; i < _INST_NUM; ++i) {
        switch(updates[i].type) {
            case _VERTEX_ADDITION:
                break;
            case _VERTEX_DELETION:
                handle_vertex_deletion(updates[i].u + 1);
                break;
            case _EDGE_ADDITION:
                handle_edge_addition(updates[i].u + 1, updates[i].v + 1);
                break;
            case _EDGE_DELETION:
                handle_edge_deletion(updates[i].u + 1, updates[i].v + 1);
                break;
            default:
                break;
        }
    }

    printf("Update time: %lld, %d\n", time, mis);

    check_mis();
    check_swap();
}

void graph::handle_vertex_deletion(unsigned int u) {
#ifndef _LINUX_
    struct timeval start, end;
    gettimeofday(&start, NULL);
#endif
    if (nodes[u].node_status == _MIS) {
        vector<unsigned int> I;
#ifdef _TWO_SWAP_
        unsigned int v = 0;
#endif
        if (one_swapable_vertex(u, I) >= 1) {
            vector<unsigned int> v_out = {u};
            swap(v_out, I);
        }
#ifdef _TWO_SWAP_
        else if (two_swapable_vertex(u, v, I) >= 2) {
            vector<unsigned int> v_out = {u, v};
            swap(v_out, I);
        }
#endif
        else {
            nodes[u].node_status = _DELETED;
            mis --;
            edge* p_edge = nodes[u].edges;
            while (p_edge != NULL) {
                unsigned int w = p_edge->node_id;
                if (nodes[w].node_status != _MIS) {
                    nodes[w].counter --;
                    /* if (nodes[w].counter == 0) {
                        mis ++;
                        add_into_IS(w);
                    } */
                }
                p_edge = p_edge->next_edge;
            }
            p_edge = nodes[u].edges;
            while (p_edge != NULL) {
                unsigned int w = p_edge->node_id;
                if (nodes[w].counter == 1) {
                    unsigned int x = 0;
                    vector<unsigned int> I;
                    edge* p_edge_w = nodes[w].edges;
                    while (p_edge_w != NULL) {
                        if (nodes[p_edge_w->node_id].node_status == _MIS) {
                            x = p_edge_w->node_id;
                            break;
                        }
                        p_edge_w = p_edge_w->next_edge;
                    }
#ifndef NDEBUG
                    if (x == 0) {
                        cout << "counter wrong detected in handle_update vertex deletion.\n";
                        exit(1);
                    }
#endif
                    if (one_swapable_vertex(x, I) > 1) {
                        vector<unsigned int> v_out = {x};
                        swap(v_out, I);
                    }
                }
#ifdef _TWO_SWAP_
                else if (nodes[w].counter == 2) {
                    unsigned int x = 0, y = 0;
                    vector<unsigned int> I;
                    edge* p_edge_w = nodes[w].edges;
                    while (p_edge_w != NULL) {
                        if (nodes[p_edge_w->node_id].node_status == _MIS) {
                            if (!x)
                                x = p_edge_w->node_id;
                            else {
                                y = p_edge_w->node_id;
                                break;
                            }
                        }
                        p_edge_w = p_edge_w->next_edge;
                    }
#ifndef NDEBUG
                    if (x == 0 || y == 0) {
                        cout << "counter wrong detected in handle_update vertex deletion.\n";
                        exit(1)
                    }
#endif
                    if (two_swapable_vertex(x, y, I) > 2) {
                        vector<unsigned int> v_out = {x, y};
                        swap(v_out, I);
                    }

                }
#endif
                p_edge = p_edge->next_edge;
            }
        }
    }
    delete_vertex(u);
}


void graph::handle_edge_addition(unsigned int u, unsigned int v) {
#ifdef _LINUX_
    struct timeval start, end;
    gettimeofday(&start, NULL);
#endif

    if (nodes[u].node_status == _NOMIS && nodes[v].node_status == _MIS)
        std::swap(u, v);
    add_edge(u, v);
    if (nodes[u].node_status == _MIS && nodes[v].node_status == _MIS) {
        nodes[u].node_status = _CONFLICT;
        nodes[u].counter = 1;
        nodes[v].node_status = _CONFLICT;
        nodes[v].counter = 1;
        vector<unsigned int> I_u, I_v, I;
        unsigned int size_u = one_swapable_vertex(u, I_u);
        unsigned int size_v = one_swapable_vertex(v, I_v);
        // there may be swapable vertex in IS
        if (max(size_u, size_v) >= 1) {
            if (size_u < size_v) {
                std::swap(u, v);
                I_u.swap(I_v);
            }
            vector<unsigned int> v_out = {u};
            swap(v_out, I_u);
            I_v.clear();
            nodes[v].node_status = _MIS;
            if (one_swapable_vertex(v, I_v) > 1) {
                v_out.clear();
                v_out.push_back(v);
                swap(v_out, I_v);
            }
        }
#ifdef _TWO_SWAP_
        else if (two_swapable_vertex(u, v, I) >= 2) {
            vector<unsigned int> v_out = {u, v};
            swap(v_out, I);
        }
#endif
        else {
            // there is no swapable vertex in IS
            // remove the vertex with higher degree from IS

            if (nodes[u].degree < nodes[v].degree)
                std::swap(u, v);
            nodes[u].counter = 1;
            nodes[u].node_status = _NOMIS;
            nodes[v].node_status = _MIS;
            nodes[v].counter = 0;
            mis --;
            edge* p_edge = nodes[u].edges;
            while (p_edge != NULL) {
                unsigned int w = p_edge->node_id;
                if (nodes[w].node_status != _MIS) {
                    nodes[w].counter --;
                    /* if (nodes[w].counter == 0) {
                        mis ++;
                        cout << w << " counter = 0.\n";
                        add_into_IS(w);
                    } */
                }
                p_edge = p_edge->next_edge;
            }
            p_edge = nodes[u].edges;
            while (p_edge != NULL) {
                unsigned int w = p_edge->node_id;
                if (nodes[w].counter == 1) {
                    unsigned int x = 0;
                    vector<unsigned int> I;
                    edge* p_edge_w = nodes[w].edges;
                    while (p_edge_w != NULL) {
                        if (nodes[p_edge_w->node_id].node_status == _MIS) {
                            x = p_edge_w->node_id;
                            break;
                        }
                        p_edge_w = p_edge_w->next_edge;
                    }
#ifndef NDEBUG
                    if (x == 0) {
                        cout << "counter wrong detected in handle_update edge addition.\n";
                        exit(1);
                    }
#endif
                    if (one_swapable_vertex(x, I) > 1) {
                        vector<unsigned int> v_out = {x};
                        swap(v_out, I);
                    }

                }
#ifdef _TWO_SWAP_
                else if (nodes[w].counter == 2) {
                    unsigned int x = 0, y = 0;
                    vector<unsigned int> I;
                    edge* p_edge_w = nodes[w].edges;
                    while (p_edge_w != NULL) {
                        if (nodes[p_edge_w->node_id].node_status == _MIS) {
                            if (!x)
                                x = p_edge_w->node_id;
                            else {
                                y = p_edge_w->node_id;
                                break;
                            }
                        }
                        p_edge_w = p_edge_w->next_edge;
                    }
#ifndef NDEBUG
                    if (x == 0 || y == 0) {
                        cout << "counter wrong detected in handle_update vertex deletion.\n";
                        exit(1)
                    }
#endif
                    if (two_swapable_vertex(x, y, I) > 2) {
                        vector<unsigned int> v_out = {x, y};
                        swap(v_out, I);
                    }

                }
#endif

                p_edge = p_edge->next_edge;
            }

        }
    } else if (nodes[u].node_status == _MIS && nodes[v].node_status == _NOMIS)
        nodes[v].counter ++;

#ifdef _LINUX_
    gettimeofday(&end, NULL);
    long long mtime, seconds, useconds;
    seconds = end.tv_sec - start.tv_sec;
    useconds = end.tv_usec - start.tv_usec;
    mtime = seconds*1000000 + useconds;
    time += mtime;
#endif

}

void graph::handle_edge_deletion(unsigned int u, unsigned int v) {
#ifdef _LINUX_
    struct timeval start, end;
    gettimeofday(&start, NULL);
#endif
    if (nodes[u].node_status == _NOMIS && nodes[v].node_status == _MIS)
        std::swap(u, v);
    delete_edge(u, v);
    if (nodes[u].node_status == _MIS && nodes[v].node_status == _NOMIS) {
        nodes[v].counter --;
        if (nodes[v].counter == 0) {
            mis ++;
            add_into_IS(v);
        } else if (nodes[v].counter == 1) {
            unsigned int w = 0;
            vector<unsigned int> I;
            edge* p_edge = nodes[v].edges;
            while (p_edge != NULL) {
                if (nodes[p_edge->node_id].node_status == _MIS) {
                    w = p_edge->node_id;
                    break;
                }
                p_edge = p_edge->next_edge;
            }
#ifndef NDEBUG
            if (w == 0) {
                cout << "counter wrong detected in handle_update edge deletion.\n";
                exit(1);
            }
#endif
            if (one_swapable_vertex(w, I) > 1) {
                vector<unsigned int> v_out = {w};
                swap(v_out, I);
            }
        }
#ifdef _TWO_SWAP_
        else if (nodes[v].counter == 2) {
            unsigned int x = 0, y = 0;
            vector<unsigned int> I;
            edge* p_edge = nodes[v].edges;
            while (p_edge != NULL) {
                if (nodes[p_edge->node_id].node_status == _MIS) {
                    if (!x)
                        x = p_edge->node_id;
                    else {
                        y = p_edge->node_id;
                        break;
                    }
                }
                p_edge = p_edge->next_edge;
            }
#ifndef NDEBUG
            if (x == 0 || y == 0) {
                cout << "counter wrong detected in handle_update vertex deletion.\n";
                exit(1)
            }
#endif
            if (two_swapable_vertex(x, y, I) > 2) {
                vector<unsigned int> v_out = {x, y};
                swap(v_out, I);
            }
        }
#endif
    } else {
        if (nodes[v].counter == 1 && nodes[u].counter == 1) {
            unsigned int w = 0;
            vector<unsigned int> I;
            edge* p_edge = nodes[v].edges;
            while (p_edge != NULL) {
                if (nodes[p_edge->node_id].node_status == _MIS) {
                    w = p_edge->node_id;
                    break;
                }
                p_edge = p_edge->next_edge;
            }
#ifndef NDEBUG
            if (w == 0) {
                cout << "counter wrong detected in handle_update edge deletion.\n";
                exit(1);
            }
#endif
            if (one_swapable_vertex(w, I) > 1) {
                vector<unsigned int> v_out = {w};
                swap(v_out, I);
            }
        }
#ifdef _TWO_SWAP_
        else if (nodes[v].counter == 2 && nodes[u].counter == 2) {
            unsigned int x = 0, y = 0;
            vector<unsigned int> I;
            edge* p_edge = nodes[v].edges;
            while (p_edge != NULL) {
                if (nodes[p_edge->node_id].node_status == _MIS) {
                    if (!x)
                        x = p_edge->node_id;
                    else {
                        y = p_edge->node_id;
                        break;
                    }
                }
                p_edge = p_edge->next_edge;
            }
#ifndef NDEBUG
            if (x == 0 || y == 0) {
                cout << "counter wrong detected in handle_update vertex deletion.\n";
                exit(1)
            }
#endif
            if (two_swapable_vertex(x, y, I) > 2) {
                vector<unsigned int> v_out = {x, y};
                swap(v_out, I);
            }
        }
#endif
    }
#ifdef _LINUX_
    gettimeofday(&end, NULL);
    long long mtime, seconds, useconds;
    seconds = end.tv_sec - start.tv_sec;
    useconds = end.tv_usec - start.tv_usec;
    mtime = seconds*1000000 + useconds;
    time += mtime;
#endif
}

void graph::check_mis() {
    int cnt = 0;
    bool maximal = true;
    // cout << "in check mis.\n";
    for (int i = 1; i <= n; ++i) {
        // std::cout << i << std::endl;
        if (nodes[i].node_status == _MIS) {
            cnt ++;
            edge* p_edge = nodes[i].edges;
            while (p_edge != NULL) {
                if (nodes[p_edge->node_id].node_status == _MIS)
                    cout << "CHECK_IS: WA two adjacent vertices in MIS.\n";
                p_edge = p_edge->next_edge;
            }
        } else if (nodes[i].node_status == _NOMIS) {
            int mis_cnt = 0;
            bool find = false;
            edge* p_edge = nodes[i].edges;
            while (p_edge != NULL) {
                if (nodes[p_edge->node_id].node_status == _MIS) {
                    find = true;
                    mis_cnt ++;
                }
                p_edge = p_edge->next_edge;
            }
            if (!find)
                cout << "CHECK_IS: WA not maximal.\n";
            if (mis_cnt != nodes[i].counter)
                cout << "CHECK_IS: vertex " << i << " wrong counter.\n";
        }
    }
    cout << "|MIS| " << cnt << ", " << mis << endl;
}

void graph::check_swap() {
    int swap_cnt = 0;
    int swap_cnt_2 = 0;
    for (int i = 1; i <= n; ++i) {
        if (nodes[i].node_status == _MIS) {
            vector<unsigned int> I;
            if (one_swapable_vertex(nodes[i].node_id, I) > 1) {
                cout << nodes[i].node_id << " degree: " << nodes[i].degree
                    << " node status: " << nodes[i].node_status
                    << " counter: " << nodes[i].counter << " adjacent list : ";
                edge* p_edge = nodes[i].edges;
                while (p_edge != NULL) {
                    cout << p_edge->node_id << ", ";
                    p_edge = p_edge->next_edge;
                }
                cout << endl;
                swap_cnt ++;
                // vector<unsigned int> v_out = {nodes[i].node_id};
                // swap(v_out, I);
            }
            I.clear();
            unsigned int v = 0;
            if (two_swapable_vertex(nodes[i].node_id, v, I) > 2) {
                swap_cnt_2 ++;
                vector<unsigned int> v_out = {nodes[i].node_id, v};
                swap(v_out, I);
            }
        }
    }
    cout << "CHECK_SWAP: |swapable| " << swap_cnt << endl;
    cout << "CHECK_SWAP: after swap |MIS| " << mis << endl;
    cout << "CHECK_SWAP: |2-swapable| " << swap_cnt_2 << endl;
    cout << "CHECK_SWAP: after 2-swap |MIS| " << mis << endl;
}

void graph::add_into_IS(unsigned int u) {
    nodes[u].node_status = _MIS;
    nodes[u].counter = 0;

    edge* p_edge = nodes[u].edges;
    unsigned int v = 0;
    while (p_edge != NULL) {
        v = p_edge->node_id;
#ifndef NDEBUG
        if (nodes[v].node_status == _MIS)
            cout << "WA: two adjacent vertices " << u << ", " << v << " in MIS.\n";
#endif
        nodes[v].node_status = _NOMIS;
        nodes[v].counter ++;
        p_edge = p_edge->next_edge;
    }
}

void graph::remove_from_IS(unsigned int u) {
    nodes[u].counter = 0;
    if (nodes[u].node_status == _CONFLICT) nodes[u].counter = 1;
    nodes[u].node_status = _NOMIS;

    edge* p_edge = nodes[u].edges;
    while (p_edge != NULL) {
        nodes[p_edge->node_id].counter --;
        p_edge = p_edge->next_edge;
    }
}

void graph::swap(const vector<unsigned int>& v_out, const vector<unsigned int>& v_in) {
    for (auto v : v_out)
        remove_from_IS(v);

    for (auto v : v_in)
        add_into_IS(v);

    mis += (v_in.size() - v_out.size());
}


void graph::add_node(unsigned int index, unsigned int node_id) {
    if (index <= 0 || index > n) {
        cout << "array out of range.\n";
        exit(1);
    }
    nodes[index] = node(node_id);
}

void graph::add_vertex(const unsigned int& u) {
    if (u <= 0 || u > n + _INST_NUM) {
        cout << "vertex number " << u << " our of range.\n";
        return;
    }
}

void graph::delete_vertex(const unsigned int& u) {
#ifndef NDEBUG
    if (u <= 0 || u > n || nodes[u].node_status == _DELETED) {
        cout << "vertex " << u << " does not exist.\n";
        return;
    }
#endif
    edge* p_edge = nodes[u].edges;
    while (p_edge != NULL) {
        unsigned int v = p_edge->node_id;
        p_edge = p_edge->next_edge;
        delete_edge(u, v);
    }
    nodes[u].node_status = _DELETED;
    nodes[u].counter = 0;
}

void graph::add_edge(unsigned int u, unsigned int v) {
#ifndef NDEBUG
    if (u <= 0 || u > n || v <= 0 || v > n) {
        cout << "vertices " << u << " or " << v << "does not exist.\n";
        return;
    }
    if (nodes[u].node_status == _DELETED || nodes[v].node_status == _DELETED) {
        cout << "vertices " << u << " or " << v << "is deleted.\n";
        return;
    }
#endif

    nodes[u].degree ++;
    edge* p_newedge = new edge(v);
    if (nodes[u].edges == NULL) nodes[u].edges = p_newedge;
    else {
        edge* p_edge = nodes[u].edges;
        edge* p_pre_edge = p_edge;
        while (p_edge != NULL && p_edge->node_id < v) {
            p_pre_edge = p_edge;
            p_edge = p_edge->next_edge;
        }
        p_newedge->next_edge = p_edge;
        if (p_pre_edge == p_edge)
            nodes[u].edges = p_newedge;
        else
            p_pre_edge->next_edge = p_newedge;
    }

    nodes[v].degree ++;
    p_newedge = new edge(u);
    if (nodes[v].edges == NULL) nodes[v].edges = p_newedge;
    else {
        edge* p_edge = nodes[v].edges;
        edge* p_pre_edge = p_edge;
        while (p_edge != NULL && p_edge->node_id < u) {
            p_pre_edge = p_edge;
            p_edge = p_edge->next_edge;
        }
        p_newedge->next_edge = p_edge;
        if (p_pre_edge == p_edge)
            nodes[v].edges = p_newedge;
        else
            p_pre_edge->next_edge = p_newedge;
    }

}

void graph::delete_edge(unsigned int u, unsigned int v) {
#ifndef NDEBUG
    if (u <= 0 || u > n || v <= 0 || v > n) {
        cout << "vertices " << u << " or " << v << "does not exist.\n";
        return;
    }
    if (nodes[u].node_status == _DELETED || nodes[v].node_status == _DELETED) {
        cout << "edge (" << u << ", " << v << ") does not exits.\n";
        return;
    }
#endif
    edge* p_edge = nodes[u].edges;
    edge* p_pre_edge = p_edge;
    while (p_edge != NULL) {
        if (p_edge->node_id == v) {
            if (p_pre_edge == p_edge)
                nodes[u].edges = p_edge->next_edge;
            else
                p_pre_edge->next_edge = p_edge->next_edge;
            delete p_edge;
            nodes[u].degree --;
            // cout << "delete edge: (" << u << ", " << v << ")\n";
            break;
        }
        p_pre_edge = p_edge;
        p_edge = p_edge->next_edge;
    }
#ifndef NDEBUG
    if (p_edge == NULL) {
        cout << "edge (" << u << ", " << v << ") does not exist.\n";
        return;
    }
#endif

    p_edge = nodes[v].edges;
    p_pre_edge = p_edge;
    while (p_edge != NULL) {
        if (p_edge->node_id == u) {
            if (p_pre_edge == p_edge)
                nodes[v].edges = p_edge->next_edge;
            else
                p_pre_edge->next_edge = p_edge->next_edge;
            delete p_edge;
            nodes[v].degree--;
            // cout << "delete edge: (" << v << ", " << u << ")\n";
            break;
        }
        p_pre_edge = p_edge;
        p_edge = p_edge->next_edge;
    }
#ifndef NDEBUG
    if (p_edge == NULL) {
        cout << "edge (" << u << ", " << v << ") does not exist.\n";
        return;
    }
#endif
}

void graph::greedy(vector<unsigned int>& I) {
    unsigned int* degree_buckets = new unsigned int[n];
    memset(degree_buckets, 0, sizeof(unsigned int)*n);
    for (int i = 1; i <= n; ++i)
        degree_buckets[nodes[i].degree]++;

    for (int i = 1; i < n; ++i)
        degree_buckets[i] += degree_buckets[i - 1];

    unsigned int* order = new unsigned int[n + 1];
    for (int i = 1; i <= n; ++i) {
        order[i] = degree_buckets[nodes[i].degree];
        degree_buckets[nodes[i].degree]--;
    }

    unsigned int* greedy_order = new unsigned int[n + 1];
    for (int i = 1; i <= n; ++i) {
        greedy_order[order[i]] = i;
    }
    for (int i = 1; i <= n; ++i) {
        if (nodes[greedy_order[i]].node_status == _UNVISITED) {
            nodes[greedy_order[i]].node_status = _MIS;
            I.push_back(nodes[greedy_order[i]].node_id);
            edge* p_edge = nodes[greedy_order[i]].edges;
            while (p_edge != NULL) {
                nodes[p_edge->node_id].node_status = _NOMIS;
                p_edge = p_edge->next_edge;
            }
        }
    }

    delete[] degree_buckets;
    delete[] order;
    delete[] greedy_order;

}

void graph::greedy_dynamic(vector<unsigned int>& I) {
    unsigned int* degree = new unsigned int[n + 1];
    for (int i = 1; i <= n; ++ i) {
        degree[i] = nodes[i].degree;
    }

    unsigned int* degree_buckets = new unsigned int[n];
    memset(degree_buckets, 0, sizeof(unsigned int)*n);
    for (int i = 1; i <= n; ++ i)
        degree_buckets[degree[i]] ++;

    for (int i = 1; i < n; ++ i)
        degree_buckets[i] += degree_buckets[i - 1];

    // order[i]  = j means that the order of node i is j
    unsigned int* order = new unsigned int[n + 1];
    for (int i = 1; i <= n; ++ i) {
        order[i] = degree_buckets[degree[i]];
        degree_buckets[degree[i]] --;
    }

    // no[i] = j means that No.i is node j
    unsigned int* no = new unsigned int[n + 1];
    for (int i = 1; i <= n; ++ i) no[order[i]] = i;

    // degree_starts[i] = j means that the first vertex with degree i is at j in no
    unsigned int* degree_starts = new unsigned int[n + 1];
    unsigned int i = 0, j = 1;
    while (j <= n) {
        degree_starts[i] = j;
        while (j<=n && degree[no[j]] == i) {
            ++ j;
        }
        ++ i;
    }
    degree_starts[i] = j;

    int res = 0;
    // | d = 1 | d = 2 | ... | d = n |
    // 当d = i的点的邻居被删除时，改点度减一，移动到d = i的队首，将degree_starts[i]后移
    for (int i = 1; i <= n; ++ i) {
        unsigned int u = no[i];
        degree_starts[degree[u]] = i + 1;
        if (nodes[u].node_status != _NOMIS) continue;
        nodes[u].node_status = _MIS;
        res ++;
        I.push_back(nodes[u].node_id);
        edge* p_edge = nodes[u].edges;
        while (p_edge != NULL) {
            unsigned int v = p_edge->node_id;
            // nodes[v].counter++;
            if (nodes[v].node_status == _UNVISITED) {
                nodes[v].node_status = _NOMIS;
                edge* p_edge_v = nodes[v].edges;
                while (p_edge_v != NULL) {
                    unsigned int w = p_edge_v->node_id;
                    if (nodes[w].node_status == _UNVISITED && w != u) {
                        // 把w移动到和它度一样元素的首位，即和位于ds位置的元素互换位置
                        unsigned ds = degree_starts[degree[w]];
                        order[no[ds]] = order[w];
                        std::swap(no[ds], no[order[w]]);
                        order[w] = ds;
                        degree_starts[degree[w]] ++;
                        degree[w] --;
                        // 如果再向前交换，交换的位置不能小于i + 1，否则改点不会被处理
                        if (degree_starts[degree[w]] <= i) degree_starts[degree[w]] = i + 1;
                    }
                    p_edge_v = p_edge_v->next_edge;
                }
            }
            p_edge = p_edge->next_edge;
        }
    }

    delete[] degree;
    delete[] degree_buckets;
    delete[] order;
    delete[] no;
    delete[] degree_starts;
}

int graph::one_swapable_vertex(unsigned int u, vector<unsigned int>& I) {
    vector<unsigned int> V;
    edge* p_edge = nodes[u].edges;
    while (p_edge != NULL) {
        if (nodes[p_edge->node_id].counter == 1 &&
                nodes[p_edge->node_id].node_status == _NOMIS) {
            V.push_back(p_edge->node_id);
        }
        p_edge = p_edge->next_edge;
    }

    if (V.size() <= 1) {
        I.swap(V);
        return I.size();
    }

    unsigned int* index = new unsigned int[n + 1];
    memset(index, 0, sizeof(unsigned int)*(n + 1));
    for (int i = 0; i < V.size(); ++ i) index[V[i]] = i + 1;

    // construct subgraph G[V]
    unsigned int subgraph_n = V.size();
    graph subgraph(subgraph_n);
    for (int i = 0; i < subgraph_n; ++ i)
        subgraph.add_node(i + 1, (unsigned int)V[i]);
    for (int i = 0; i < subgraph_n; ++ i) {
        edge* p_edge = nodes[V[i]].edges;
        int j = i + 1;
        while (p_edge != NULL && j < V.size()) {
            if (p_edge->node_id > V[i]) {
                if (p_edge->node_id > V[j]) j ++;
                else if (p_edge->node_id < V[j]) p_edge = p_edge->next_edge;
                else {
#ifndef NDEBUG
                    if (index[p_edge->node_id] == 0)
                        cout << "node " << p_edge->node_id << " is not in V\n";
#endif
                    subgraph.add_edge(i + 1, index[p_edge->node_id]);
                    j ++;
                    p_edge = p_edge->next_edge;
                }
            } else {
                p_edge = p_edge->next_edge;
            }
        }
    }
    delete[] index;

    // subgraph.greedy_dynamic(I);
    subgraph.greedy(I);

    return I.size();
}

int graph::two_swapable_vertex(unsigned int u, unsigned int& v, vector<unsigned int>& I) {
    unordered_map<unsigned int, vector<unsigned int> > V;
    edge* p_edge = nodes[u].edges;

    while (p_edge != NULL) {
        if (nodes[p_edge->node_id].counter == 2 &&
                nodes[p_edge->node_id].node_status == _NOMIS) {
            unsigned int w = 0;
            edge* p_edge_tmp = nodes[p_edge->node_id].edges;
            while (p_edge_tmp != NULL) {
                if (nodes[p_edge_tmp->node_id].node_status == _MIS &&
                        p_edge_tmp->node_id != u) {
                    w = p_edge_tmp->node_id;
                    break;
                }
                p_edge_tmp = p_edge_tmp->next_edge;
            }
#ifndef NDEBUG
            if (w == 0) {
                cout << "didn't find the second IS-neighbor.\n";
                exit(1);
            }
#endif
            if (V.count(w) != 0)
                V[w].push_back(p_edge->node_id);
            else {
                V[w] = vector<unsigned int>();
                V[w].push_back(p_edge->node_id);
            }
        }
        p_edge = p_edge->next_edge;
    }

    int size = 0;
    if (v == 0) {
        for (auto it : V) {
            v = it.second.size() > size ? it.first : v;
            size = max(size, (int)it.second.size());
        }
    } else
        size = V[v].size();

    unsigned int* index = new unsigned int[n + 1];
    memset(index, 0, sizeof(unsigned int)*(n + 1));
    for (int i = 0; i < V[v].size(); ++ i) index[V[v][i]] = i + 1;

    unsigned int subgraph_n = size;
    graph subgraph(subgraph_n);
    for (int i = 0; i < subgraph_n; ++ i)
        subgraph.add_node(i + 1, (unsigned int)V[v][i]);

    for (int i = 0; i < subgraph_n; ++ i) {
        edge* p_edge = nodes[V[v][i]].edges;
        int j = i + 1;
        while (p_edge != NULL && j < size) {
            if (p_edge->node_id > V[v][i]) {
                if (p_edge->node_id > V[v][j]) j ++;
                else if (p_edge->node_id < V[v][j]) p_edge = p_edge->next_edge;
                else {
#ifndef NDEBUG
                    if (index[p_edge->node_id] == 0)
                        cout << "node " << p_edge->node_id << " is not in V.\n";
#endif
                    subgraph.add_edge(i + 1, index[p_edge->node_id]);
                    j ++;
                    p_edge = p_edge->next_edge;
                }
            } else {
                p_edge = p_edge->next_edge;
            }
        }
    }

    delete[] index;

    subgraph.greedy(I);

    return I.size();
}


/*
 * test for graph.h
 */
int main(int argc, char *argv[])
{
    // std::cout << argv[1] << std::endl;
    cout << argc << endl;
    if (argc == 2) {
        string file_path = (string)argv[1] + ".bin";
        string mis_path = (string)argv[1] + ".mis";
        string inst_path = (string)argv[1] + ".inst";
        graph g(file_path.c_str(), mis_path.c_str());
        g.read_graph();
        g.read_mis();
        g.experiment(inst_path.c_str());
    } else if (argc == 4) {
        graph g(argv[1], argv[2]);
        g.read_graph();
        g.read_mis();
        g.experiment(argv[3]);
    }
    else {
        cout << "error.\n";
    }
    return 0;
}
