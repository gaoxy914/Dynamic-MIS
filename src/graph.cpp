#include "graph.h"

graph::graph(const char* file_path, const char * mis_path) {
    _file_path = string(file_path);
    _mis_path = string(mis_path);
    _n = _m = _mis = _utime = 0;
    _nodes = NULL;
}

graph::graph(ui n) {
    _file_path = _mis_path = "";
    _n = n;
    _m = _mis = 0;
    _nodes = NULL;
    _nodes = new node_t[_n + 1];
}

graph::~graph() {
    if (_nodes != NULL) {
        delete[] _nodes;
        _nodes = NULL;
    }
}

/*
 * graph file format
 * _n m
 * u v // m lines such that (u, v) in E
 */
void graph::read_graph() {
    ifstream infile((_GRAPH_PATH + _file_path).c_str());
    if (!infile.is_open()) {
        cout << "file: " << _file_path << " open failed." << endl;
        exit(1);
    }

    ui m = 0;
    infile >> _n >> m;
    cout << "n = " << _n << "; m = " << m << endl;

    if (_nodes == NULL) _nodes = new node_t[_n + 1];
    for (ui i = 1; i <= _n; ++ i) {
        _nodes[i] = node_t(i);
    }
    ui u, v;
    for (ui i = 0; i < m; ++ i) {
        infile >> u >> v;
        _add_edge(u + 1, v + 1);
    }

#ifndef NDEBUG
    long long sum_degree = 0;
    for (ui i = 1; i <= _n; ++ i) {
        sum_degree += _nodes[i].degree;
    }
    if (sum_degree != 2*m) {
        cout << sum_degree << ", " << 2*m << endl;
        cout << "wrong in input graph.\n";
    }
#endif
    infile.close();
}

/*
 * mis file format
 * mis
 * u # u in IS
 */
void graph::read_mis() {
    ifstream infile((_MIS_PATH + _mis_path).c_str());
    if (!infile.is_open()) {
        cout << "file: " << _mis_path << " open failed." << endl;
        exit(1);
    }
    infile >> _mis;
    ui u;
    for (ui i = 0; i < _mis; ++i) {
        infile >> u;
        _add_into_IS(u + 1);
    }
    infile.close();
}

void graph::show() {
    for (ui i = 1; i <= _n; ++i) {
        cout << _nodes[i].node_id << " degree: " << _nodes[i].degree
            << " node status: " << _nodes[i].node_status
            << " counter: " << _nodes[i].counter << " adjacent list : ";

        edge_t* p_edge = _nodes[i].edges;
        while (p_edge != NULL) {
            if (_nodes[p_edge->node_id].node_status != _DELETED)
                cout << p_edge->node_id << ", ";
            p_edge = p_edge->next_edge;
        }
        cout << endl;
    }
}

void graph::greedy() {
    ui* degree_buckets = new ui[_n];
    memset(degree_buckets, 0, sizeof(ui)*_n);
    for (ui i = 1; i <= _n; ++i)
        degree_buckets[_nodes[i].degree]++;

    for (ui i = 1; i < _n; ++i)
        degree_buckets[i] += degree_buckets[i - 1];

    // order[i]  = j means that the order of node_t i is j
    ui* order = new ui[_n + 1];
    for (ui i = 1; i <= _n; ++i) {
        order[i] = degree_buckets[_nodes[i].degree];
        degree_buckets[_nodes[i].degree]--;
    }

    // greedy_order[i] = j means that No.i is node_t j
    ui* greedy_order = new ui[_n + 1];
    for (ui i = 1; i <= _n; ++i) {
        greedy_order[order[i]] = i;
    }

    for (ui i = 1; i <= _n; ++i) {
        if (_nodes[greedy_order[i]].node_status == _UNVISITED) {
            _mis ++;
            _add_into_IS(greedy_order[i]);
        }
    }

    cout << "Greedy MIS: " << _mis << endl;

    delete[] degree_buckets;
    delete[] order;
    delete[] greedy_order;

#ifndef NDEBUG
    _check_mis();
#endif
}

void graph::experiment(const char* inst_path) {
    vector<update_t> updates;
    ifstream infile((_INST_PATH + (string)inst_path).c_str());
    if (!infile.is_open()) {
        cout << "instruction file: " << inst_path << " open failed." << endl;
        exit(1);
    }
    update_t update;
    for (ui i = 0; i < _INST_NUM; ++i) {
        infile >> update.type;
        switch(update.type) {
        case _VERTEX_ADDITION:
            infile >> update.u;
            break;
        case _VERTEX_DELETION:
            infile >> update.u;
            break;
        case _EDGE_ADDITION:
            infile >> update.u >> update.v;
            break;
        case _EDGE_DELETION:
            infile >> update.u >> update.v;
            break;
        default:
            break;
        }
        updates.push_back(update);
    }
    infile.close();

    for (ui i = 0; i < _INST_NUM; ++i) {
#ifndef NDEBUG
        cout << i << ", " << updates[i].type << ", " << updates[i].u + 1 << "," << updates[i].v + 1 << endl;
#endif
        // _simple(updates[i]);
        switch(updates[i].type) {
            case _VERTEX_ADDITION:
                break;
            case _VERTEX_DELETION:
                _handle_vertex_deletion(updates[i].u + 1);
                break;
            case _EDGE_ADDITION:
                _handle_edge_addition(updates[i].u + 1, updates[i].v + 1);
                break;
            case _EDGE_DELETION:
                _handle_edge_deletion(updates[i].u + 1, updates[i].v + 1);
                break;
            default:
                break;
        }
#ifndef NDEBUG
        _check_mis();
#endif
    }

    cout << "Update time: " << _utime << ", |MIS| " << _mis << endl;
    _check_mis();
    // _check_swap();
}

void graph::experiment2(const char *inst_path) {
    cout << "SCALABILITY.\n";
    vector<update_t> updates;
    ifstream infile((_INST_PATH + (string)inst_path).c_str());
    if (!infile.is_open()) {
        cout << "instruction file: " << inst_path << " open failed." << endl;
        exit(1);
    }
    // _check_swap();

    update_t update;
    for (ui i = 0; i < _INST_NUM; ++i) {
        infile >> update.type;
        switch(update.type) {
        case _VERTEX_ADDITION:
            infile >> update.u;
            break;
        case _VERTEX_DELETION:
            infile >> update.u;
            break;
        case _EDGE_ADDITION:
            infile >> update.u >> update.v;
            break;
        case _EDGE_DELETION:
            infile >> update.u >> update.v;
            break;
        default:
            break;
        }
        updates.push_back(update);
    }
    infile.close();

    for (ui i = 0; i < 10; ++ i) {
        cout << i << endl;
        for (ui j = i*1000; j < (i + 1)*1000; ++ j) {
            switch(updates[j].type) {
            case _VERTEX_ADDITION:
                break;
            case _VERTEX_DELETION:
                _handle_vertex_deletion(updates[j].u + 1);
                break;
            case _EDGE_ADDITION:
                _handle_edge_addition(updates[j].u + 1, updates[j].v + 1);
                break;
            case _EDGE_DELETION:
                _handle_edge_deletion(updates[j].u + 1, updates[j].v + 1);
                break;
            default:
                break;
            }
        }
        cout << "Update time: " << _utime << ", |MIS| " << _mis << endl;
        _check_mis();
        _check_swap();
    }
}

void graph::_simple(update_t update) {
#ifdef _LINUX_
    struct timeval start, end;
    gettimeofday(&start, NULL);
#endif
    ui u = update.u + 1, v = update.v + 1;
    switch (update.type) {
    case _VERTEX_ADDITION:
        break;
    case _VERTEX_DELETION:
        if (_nodes[u].node_status == _MIS) {
            _nodes[u].node_status = _DELETED;
            _mis --;
            edge_t* p_edge = _nodes[u].edges;
            while (p_edge != NULL) {
                ui w = p_edge->node_id;
                if (_nodes[w].node_status != _DELETED) {
                    _nodes[w].counter --;
                    if (_nodes[w].counter == 0) {
                        _mis ++;
                        _add_into_IS(w);
                    }
                }
                p_edge = p_edge->next_edge;
            }
        }
        _delete_vertex(u);
        break;
    case _EDGE_ADDITION:
        _add_edge(u, v);
        if (_nodes[u].node_status == _MIS && _nodes[v].node_status == _MIS) {
            _nodes[u].counter = 1;
            _nodes[u].node_status = _NOMIS;
            _nodes[v].node_status = _MIS;
            _nodes[v].counter = 0;
            _mis --;
            edge_t* p_edge = _nodes[u].edges;
            while (p_edge != NULL) {
                ui w = p_edge->node_id;
                if (_nodes[w].node_status != _DELETED) {
                    _nodes[w].counter --;
                    if (_nodes[w].counter == 0) {
                        _mis ++;
                        _add_into_IS(w);
                    }
                }
                p_edge = p_edge->next_edge;
            }
        } else if (_nodes[u].node_status == _MIS && _nodes[v].node_status == _NOMIS) {
            _nodes[v].counter ++;
        } else if (_nodes[u].node_status == _NOMIS && _nodes[v].node_status == _MIS) {
            _nodes[u].counter ++;
        }
        break;
    case _EDGE_DELETION:
        _delete_edge(u, v);
        if (_nodes[u].node_status == _NOMIS && _nodes[v].node_status == _MIS)
            std::swap(u, v);
        if (_nodes[u].node_status == _MIS && _nodes[v].node_status == _NOMIS) {
            _nodes[v].counter --;
            if (_nodes[v].counter == 0) {
                _mis ++;
                _add_into_IS(v);
            }
        }
        break;
    default:
        break;
    }
#ifdef _LINUX_
    gettimeofday(&end, NULL);
    long long mtime, seconds, useconds;
    seconds = end.tv_sec - start.tv_sec;
    useconds = end.tv_usec - start.tv_usec;
    mtime = seconds*1000000 + useconds;
    _utime += mtime;
#endif
}

void graph::_handle_vertex_deletion(ui u) {
#ifdef _LINUX_
    struct timeval start, end;
    gettimeofday(&start, NULL);
#endif

    if (_nodes[u].node_status == _MIS) {
        vector<ui> I;
#ifdef _TWO_SWAP_
        ui v = 0;
#endif
        if (_one_swapable_vertex(u, I) >= 1) {
            vector<ui> v_out = {u};
            _swap(v_out, I);
        }
#ifdef _TWO_SWAP_
        else if (_two_swapable_vertex(u, v, I) >= 2) {
            vector<ui> v_out = {u, v};
            _swap(v_out, I);
        }
#endif
        else {
            _nodes[u].node_status = _DELETED;
            _mis --;
            edge_t* p_edge = _nodes[u].edges;
            while (p_edge != NULL) {
                ui w = p_edge->node_id;
                if (_nodes[w].node_status != _DELETED) {
                    _nodes[w].counter --;
                    /* if (_nodes[w].counter == 0) {
                        _mis ++;
                        _add_into_IS(w);
                    }*/
                }
                p_edge = p_edge->next_edge;
            }
            p_edge = _nodes[u].edges;
            while (p_edge != NULL) {
                ui w = p_edge->node_id;
                if (_nodes[w].counter == 1) {
                    ui x = 0;
                    vector<ui> I;
                    edge_t* p_edge_w = _nodes[w].edges;
                    while (p_edge_w != NULL) {
                        if (_nodes[p_edge_w->node_id].node_status == _MIS) {
                            x = p_edge_w->node_id;
                            break;
                        }
                        p_edge_w = p_edge_w->next_edge;
                    }
#ifndef NDEBUG
                    if (x == 0) {
                        cout << "counter wrong detected in handle update vertex deletion.\n";
                        exit(1);
                    }
#endif
                    if (_one_swapable_vertex(x, I) > 1) {
                        vector<ui> v_out = {x};
                        _swap(v_out, I);
                    }
                }
#ifdef _TWO_SWAP_
                else if (_nodes[w].counter == 2) {
                    ui x = 0, y = 0;
                    vector<ui> I;
                    edge_t* p_edge_w = _nodes[w].edges;
                    while (p_edge_w != NULL) {
                        if (_nodes[p_edge_w->node_id].node_status == _MIS) {
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
                        cout << "counter wrong detected in handle update vertex deletion.\n";
                        exit(1);
                    }
#endif
                    if (_two_swapable_vertex(x, y, I) > 2) {
                        vector<ui> v_out = {x, y};
                        _swap(v_out, I);
                    }

                }
#endif
                p_edge = p_edge->next_edge;
            }
        }
    }
    _delete_vertex(u);
#ifdef _LINUX_
    gettimeofday(&end, NULL);
    long long mtime, seconds, useconds;
    seconds = end.tv_sec - start.tv_sec;
    useconds = end.tv_usec - start.tv_usec;
    mtime = seconds*1000000 + useconds;
    _utime += mtime;
#endif
}


void graph::_handle_edge_addition(ui u, ui v) {
#ifdef _LINUX_
    struct timeval start, end;
    gettimeofday(&start, NULL);
#endif

    if (_nodes[u].node_status == _NOMIS && _nodes[v].node_status == _MIS)
        std::swap(u, v);
    _add_edge(u, v);
    if (_nodes[u].node_status == _MIS && _nodes[v].node_status == _MIS) {
        _nodes[u].node_status = _CONFLICT;
        _nodes[u].counter = 1;
        _nodes[v].node_status = _CONFLICT;
        _nodes[v].counter = 1;
        vector<ui> I_u, I_v;
        ui size_u = _one_swapable_vertex(u, I_u);
        ui size_v = _one_swapable_vertex(v, I_v);
#ifdef _TWO_SWAP_
        vector<ui> I_ux, I_vy;
        ui x = 0, y = 0;
#endif
        // there may be swapable vertex in IS
        if (max(size_u, size_v) >= 1) {
            if (size_u < size_v) {
                std::swap(u, v);
                I_u.swap(I_v);
            }
            vector<ui> v_out = {u};
            _swap(v_out, I_u);
            I_v.clear();
            _nodes[v].node_status = _MIS;
            if (_one_swapable_vertex(v, I_v) > 1) {
                v_out.clear();
                v_out.push_back(v);
                _swap(v_out, I_v);
            }
        }
#ifdef _TWO_SWAP_
        else if (max(_two_swapable_vertex(u, x, I_ux), _two_swapable_vertex(v, y, I_vy)) >= 2) {
            if (I_ux.size() < I_vy.size()) {
                std::swap(u, v);
                std::swap(x, y);
                I_ux.swap(I_vy);
            }
            vector<ui> v_out = {u, x};
            _swap(v_out, I_ux);
            I_vy.clear();
            if (x != v) {
                _nodes[v].node_status = _MIS;
                if (_two_swapable_vertex(v, y, I_vy) > 2) {
                    v_out.clear();
                    v_out.push_back(v);
                    v_out.push_back(y);
                    _swap(v_out, I_vy);
                }
            }
        }
#endif
        else {
            // there is no swapable vertex in IS
            // remove the vertex with higher degree from IS
            if (_nodes[u].degree < _nodes[v].degree)
                std::swap(u, v);
            _nodes[u].counter = 1;
            _nodes[u].node_status = _NOMIS;
            _nodes[v].node_status = _MIS;
            _nodes[v].counter = 0;
            _mis --;
            edge_t* p_edge = _nodes[u].edges;
            while (p_edge != NULL) {
                ui w = p_edge->node_id;
                if (_nodes[w].node_status != _DELETED) {
                    _nodes[w].counter --;
                    /* if (_nodes[w].counter == 0) {
                        _mis ++;
                        _add_into_IS(w);
                    }*/
                }
                p_edge = p_edge->next_edge;
            }
            p_edge = _nodes[u].edges;
            while (p_edge != NULL) {
                ui w = p_edge->node_id;
                if (_nodes[w].counter == 1) {
                    ui x = 0;
                    vector<ui> I;
                    edge_t* p_edge_w = _nodes[w].edges;
                    while (p_edge_w != NULL) {
                        if (_nodes[p_edge_w->node_id].node_status == _MIS) {
                            x = p_edge_w->node_id;
                            break;
                        }
                        p_edge_w = p_edge_w->next_edge;
                    }
#ifndef NDEBUG
                    if (x == 0) {
                        cout << "counter wrong detected in handle update edge addition.\n";
                        exit(1);
                    }
#endif
                    if (_one_swapable_vertex(x, I) > 1) {
                        vector<ui> v_out = {x};
                        _swap(v_out, I);
                    }

                }
#ifdef _TWO_SWAP_
                else if (_nodes[w].counter == 2) {
                    ui x = 0, y = 0;
                    vector<ui> I;
                    edge_t* p_edge_w = _nodes[w].edges;
                    while (p_edge_w != NULL) {
                        if (_nodes[p_edge_w->node_id].node_status == _MIS) {
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
                        cout << "counter wrong detected in handle update vertex deletion.\n";
                        exit(1);
                    }
#endif
                    if (_two_swapable_vertex(x, y, I) > 2) {
                        vector<ui> v_out = {x, y};
                        _swap(v_out, I);
                    }

                }
#endif

                p_edge = p_edge->next_edge;
            }

        }
    } else if (_nodes[u].node_status == _MIS && _nodes[v].node_status == _NOMIS)
        _nodes[v].counter ++;

#ifdef _LINUX_
    gettimeofday(&end, NULL);
    long long mtime, seconds, useconds;
    seconds = end.tv_sec - start.tv_sec;
    useconds = end.tv_usec - start.tv_usec;
    mtime = seconds*1000000 + useconds;
    _utime += mtime;
#endif

}

void graph::_handle_edge_deletion(ui u, ui v) {
#ifdef _LINUX_
    struct timeval start, end;
    gettimeofday(&start, NULL);
#endif
    if (_nodes[u].node_status == _NOMIS && _nodes[v].node_status == _MIS)
        std::swap(u, v);
    _delete_edge(u, v);
    if (_nodes[u].node_status == _MIS && _nodes[v].node_status == _NOMIS) {
        _nodes[v].counter --;
        if (_nodes[v].counter == 0) {
            _mis ++;
            _add_into_IS(v);
        } else if (_nodes[v].counter == 1) {
            ui w = 0;
            vector<ui> I;
            edge_t* p_edge = _nodes[v].edges;
            while (p_edge != NULL) {
                if (_nodes[p_edge->node_id].node_status == _MIS) {
                    w = p_edge->node_id;
                    break;
                }
                p_edge = p_edge->next_edge;
            }
#ifndef NDEBUG
            if (w == 0) {
                cout << "counter wrong detected in handle update edge deletion.\n";
                exit(1);
            }
#endif
            if (_one_swapable_vertex(w, I) > 1) {
                vector<ui> v_out = {w};
                _swap(v_out, I);
            }
        }
#ifdef _TWO_SWAP_
        else if (_nodes[v].counter == 2) {
            ui x = 0, y = 0;
            vector<ui> I;
            edge_t* p_edge = _nodes[v].edges;
            while (p_edge != NULL) {
                if (_nodes[p_edge->node_id].node_status == _MIS) {
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
                cout << "counter wrong detected in handle update edge deletion.\n";
                exit(1);
            }
#endif
            if (_two_swapable_vertex(x, y, I) > 2) {
                vector<ui> v_out = {x, y};
                _swap(v_out, I);
            }
        }
#endif
    } else {
        if (_nodes[v].counter == 1 && _nodes[u].counter == 1) {
            ui w = 0;
            vector<ui> I;
            edge_t* p_edge = _nodes[v].edges;
            while (p_edge != NULL) {
                if (_nodes[p_edge->node_id].node_status == _MIS) {
                    w = p_edge->node_id;
                    break;
                }
                p_edge = p_edge->next_edge;
            }
#ifndef NDEBUG
            if (w == 0) {
                cout << "counter wrong detected in handle update edge deletion.\n";
                exit(1);
            }
#endif
            if (_one_swapable_vertex(w, I) > 1) {
                vector<ui> v_out = {w};
                _swap(v_out, I);
            }
        }
#ifdef _TWO_SWAP_
        /* else if (_nodes[v].counter == 2 && _nodes[u].counter == 2) {
            ui x = 0, y = 0;
            vector<ui> I;
            edge_t* p_edge = _nodes[v].edges;
            while (p_edge != NULL) {
                if (_nodes[p_edge->node_id].node_status == _MIS) {
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
                cout << "counter wrong detected in handle update edge deletion.\n";
                exit(1);
            }
#endif
            if (_two_swapable_vertex(x, y, I) > 2) {
                vector<ui> v_out = {x, y};
                _swap(v_out, I);
            }
        }*/
        else if (max(_nodes[v].counter, _nodes[u].counter) == 2) {
            if (_nodes[v].counter < 2) std::swap(u, v);
            ui x = 0, y = 0;
            vector<ui> I;
            edge_t* p_edge = _nodes[v].edges;
            while (p_edge != NULL) {
                if (_nodes[p_edge->node_id].node_status == _MIS) {
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
                cout << "counter wrong detected in handle update edge deletion.\n";
                exit(1);
            }
#endif
            if (_two_swapable_vertex(x, y, I) > 2) {
                vector<ui> v_out = {x, y};
                _swap(v_out, I);
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
    _utime += mtime;
#endif
}

void graph::_check_mis() {
    ui cnt = 0;
    bool maximal = true;
    // cout << "in check _mis.\n";
    for (ui i = 1; i <= _n; ++i) {
        // std::cout << i << std::endl;
        if (_nodes[i].node_status == _MIS) {
            cnt ++;
            edge_t* p_edge = _nodes[i].edges;
            while (p_edge != NULL) {
                if (_nodes[p_edge->node_id].node_status == _MIS) {
                    cout << "CHECK_IS: WA two adjacent vertices in MIS.\n";
                    exit(1);
                }
                p_edge = p_edge->next_edge;
            }
        } else if (_nodes[i].node_status == _NOMIS) {
            ui mis_cnt = 0;
            bool find = false;
            edge_t* p_edge = _nodes[i].edges;
            while (p_edge != NULL) {
                if (_nodes[p_edge->node_id].node_status == _MIS) {
                    find = true;
                    mis_cnt ++;
                }
                p_edge = p_edge->next_edge;
            }
            if (!find) {
                cout << "CHECK_IS: WA " << i << " not maximal.\n";
                exit(1);
            }
            if (mis_cnt != _nodes[i].counter) {
                cout << "CHECK_IS: vertex " << i << " wrong counter.\n";
                cout << mis_cnt << ", " << _nodes[i].counter << endl;
                exit(1);
            }
        } else if (_nodes[i].node_status == _DELETED) {
            if (_nodes[i].counter != 0) {
                cout << "CHECK_IS: deleted vertex " << i << " wrong contuer.\n";
                exit(1);
            }
        } else {
            cout << "CHECK_IS: vertex " << i << " statues " << _nodes[i].node_status << endl;
            exit(1);
        }
    }
    cout << "|MIS| " << cnt << ", " << _mis << endl;
}

void graph::_check_swap() {
    ui swap_cnt = 0;
    ui swap_cnt_2 = 0;
    for (ui i = 1; i <= _n; ++i) {
        if (_nodes[i].node_status == _MIS) {
            vector<ui> I;
            if (_one_swapable_vertex(_nodes[i].node_id, I) > 1) {
                swap_cnt ++;
                // vector<ui> v_out = {_nodes[i].node_id};
                // _swap(v_out, I);
            }
#ifdef _TWO_SWAP_
            I.clear();
            ui v = 0;
            if (_two_swapable_vertex(_nodes[i].node_id, v, I) > 2) {
                swap_cnt_2 ++;
                // vector<ui> v_out = {_nodes[i].node_id, v};
                // _swap(v_out, I);
            }
#endif
        }
    }
    cout << "CHECK_SWAP: |1-swapable| " << swap_cnt << endl;
    cout << "CHECK_SWAP: |2-swapable| " << swap_cnt_2 << endl;
}

void graph::_add_into_IS(ui u) {
    _nodes[u].node_status = _MIS;
    _nodes[u].counter = 0;

    edge_t* p_edge = _nodes[u].edges;
    ui v = 0;
    while (p_edge != NULL) {
        v = p_edge->node_id;
#ifndef NDEBUG
        if (_nodes[v].node_status == _MIS)
            cout << "WA: two adjacent vertices " << u << ", " << v << " in MIS.\n";
#endif
        if (_nodes[v].node_status != _DELETED) {
            _nodes[v].node_status = _NOMIS;
            _nodes[v].counter ++;
        }
        p_edge = p_edge->next_edge;
    }
}

void graph::_remove_from_IS(ui u) {
    _nodes[u].counter = 0;
    if (_nodes[u].node_status == _CONFLICT) _nodes[u].counter = 1;
    _nodes[u].node_status = _NOMIS;

    edge_t* p_edge = _nodes[u].edges;
    while (p_edge != NULL) {
        if (_nodes[p_edge->node_id].node_status != _DELETED)
            _nodes[p_edge->node_id].counter --;
        p_edge = p_edge->next_edge;
    }
}

void graph::_swap(const vector<ui>& v_out, const vector<ui>& v_in) {
    for (auto v : v_out)
        _remove_from_IS(v);

    for (auto v : v_in)
        _add_into_IS(v);

    _mis += (v_in.size() - v_out.size());
}

void graph::_add_node(ui index, ui node_id) {
#ifndef NDEBUG
    if (index <= 0 || index > _n) {
        cout << "array out of range.\n";
        exit(1);
    }
#endif
    _nodes[index] = node_t(node_id);
}

void graph::_add_vertex(const ui& u) {
    if (u <= 0 || u > _n + _INST_NUM) {
        cout << "vertex number " << u << " our of range.\n";
        return;
    }
}

void graph::_delete_vertex(const ui& u) {
#ifndef NDEBUG
    if (u <= 0 || u > _n) {
        cout << "vertex " << u << " does not exist.\n";
        return;
    }
#endif
    // _n --;
    /* edge_t* p_edge = _nodes[u].edges;
    while (p_edge != NULL) {
        ui v = p_edge->node_id;
        p_edge = p_edge->next_edge;
        _delete_edge(u, v);
    }*/
    _nodes[u].node_status = _DELETED;
    _nodes[u].counter = 0;
}

void graph::_add_edge(ui u, ui v) {
#ifndef NDEBUG
    if (u <= 0 || u > _n || v <= 0 || v > _n) {
        cout << "vertices " << u << " or " << v << " does not exist.\n";
        return;
    }
    if (_nodes[u].node_status == _DELETED || _nodes[v].node_status == _DELETED) {
        cout << "vertices " << u << " or " << v << " is deleted.\n";
        return;
    }
#endif

    _m ++;
    _nodes[u].degree ++;
    edge_t* p_newedge = new edge_t(v);
    if (_nodes[u].edges == NULL) _nodes[u].edges = p_newedge;
    else {
        edge_t* p_edge = _nodes[u].edges;
        edge_t* p_pre_edge = p_edge;
        while (p_edge != NULL && p_edge->node_id < v) {
            p_pre_edge = p_edge;
            p_edge = p_edge->next_edge;
        }
        p_newedge->next_edge = p_edge;
        if (p_pre_edge == p_edge)
            _nodes[u].edges = p_newedge;
        else
            p_pre_edge->next_edge = p_newedge;
    }

    _nodes[v].degree ++;
    p_newedge = new edge_t(u);
    if (_nodes[v].edges == NULL) _nodes[v].edges = p_newedge;
    else {
        edge_t* p_edge = _nodes[v].edges;
        edge_t* p_pre_edge = p_edge;
        while (p_edge != NULL && p_edge->node_id < u) {
            p_pre_edge = p_edge;
            p_edge = p_edge->next_edge;
        }
        p_newedge->next_edge = p_edge;
        if (p_pre_edge == p_edge)
            _nodes[v].edges = p_newedge;
        else
            p_pre_edge->next_edge = p_newedge;
    }

}

void graph::_delete_edge(ui u, ui v) {
#ifndef NDEBUG
    if (u <= 0 || u > _n || v <= 0 || v > _n) {
        cout << "vertices " << u << " or " << v << "does not exist.\n";
        return;
    }
    if (_nodes[u].node_status == _DELETED || _nodes[v].node_status == _DELETED) {
        cout << "edge (" << u << ", " << v << ") does not exits.\n";
        return;
    }
#endif
    _m --;
    edge_t* p_edge = _nodes[u].edges;
    edge_t* p_pre_edge = p_edge;
    while (p_edge != NULL) {
        if (p_edge->node_id == v) {
            if (p_pre_edge == p_edge)
                _nodes[u].edges = p_edge->next_edge;
            else
                p_pre_edge->next_edge = p_edge->next_edge;
            delete p_edge;
            _nodes[u].degree --;
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

    p_edge = _nodes[v].edges;
    p_pre_edge = p_edge;
    while (p_edge != NULL) {
        if (p_edge->node_id == u) {
            if (p_pre_edge == p_edge)
                _nodes[v].edges = p_edge->next_edge;
            else
                p_pre_edge->next_edge = p_edge->next_edge;
            delete p_edge;
            _nodes[v].degree--;
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

void graph::_greedy(vector<ui>& I) {
    ui* degree_buckets = new ui[_n];
    memset(degree_buckets, 0, sizeof(ui)*_n);
    for (ui i = 1; i <= _n; ++i)
        degree_buckets[_nodes[i].degree]++;

    for (ui i = 1; i < _n; ++i)
        degree_buckets[i] += degree_buckets[i - 1];

    ui* order = new ui[_n + 1];
    for (ui i = 1; i <= _n; ++i) {
        order[i] = degree_buckets[_nodes[i].degree];
        degree_buckets[_nodes[i].degree]--;
    }

    ui* greedy_order = new ui[_n + 1];
    for (ui i = 1; i <= _n; ++i) {
        greedy_order[order[i]] = i;
    }
    for (ui i = 1; i <= _n; ++i) {
        if (_nodes[greedy_order[i]].node_status == _UNVISITED) {
            _nodes[greedy_order[i]].node_status = _MIS;
            I.push_back(_nodes[greedy_order[i]].node_id);
            edge_t* p_edge = _nodes[greedy_order[i]].edges;
            while (p_edge != NULL) {
                _nodes[p_edge->node_id].node_status = _NOMIS;
                p_edge = p_edge->next_edge;
            }
        }
    }

    delete[] degree_buckets;
    delete[] order;
    delete[] greedy_order;

}

void graph::_greedy_dynamic(vector<ui>& I) {
    ui* degree = new ui[_n + 1];
    for (ui i = 1; i <= _n; ++ i) {
        degree[i] = _nodes[i].degree;
    }

    ui* degree_buckets = new ui[_n];
    memset(degree_buckets, 0, sizeof(ui)*_n);
    for (ui i = 1; i <= _n; ++ i)
        degree_buckets[degree[i]] ++;

    for (ui i = 1; i < _n; ++ i)
        degree_buckets[i] += degree_buckets[i - 1];

    // order[i]  = j means that the order of node_t i is j
    ui* order = new ui[_n + 1];
    for (ui i = 1; i <= _n; ++ i) {
        order[i] = degree_buckets[degree[i]];
        degree_buckets[degree[i]] --;
    }

    // no[i] = j means that No.i is node_t j
    ui* no = new ui[_n + 1];
    for (ui i = 1; i <= _n; ++ i) no[order[i]] = i;

    // degree_starts[i] = j means that the first vertex with degree i is at j in no
    ui* degree_starts = new ui[_n + 1];
    ui i = 0, j = 1;
    while (j <= _n) {
        degree_starts[i] = j;
        while (j<=_n && degree[no[j]] == i) {
            ++ j;
        }
        ++ i;
    }
    degree_starts[i] = j;

    ui res = 0;
    // | d = 1 | d = 2 | ... | d = _n |
    // 当d = i的点的邻居被删除时，改点度减一，移动到d = i的队首，将degree_starts[i]后移
    for (ui i = 1; i <= _n; ++ i) {
        ui u = no[i];
        degree_starts[degree[u]] = i + 1;
        if (_nodes[u].node_status == _NOMIS) continue;
        _nodes[u].node_status = _MIS;
        res ++;
        I.push_back(_nodes[u].node_id);
        edge_t* p_edge = _nodes[u].edges;
        while (p_edge != NULL) {
            ui v = p_edge->node_id;
            // _nodes[v].counter++;
            if (_nodes[v].node_status == _UNVISITED) {
                _nodes[v].node_status = _NOMIS;
                edge_t* p_edge_v = _nodes[v].edges;
                while (p_edge_v != NULL) {
                    ui w = p_edge_v->node_id;
                    if (_nodes[w].node_status == _UNVISITED && w != u) {
                        // 把w移动到和它度一样元素的首位，即和位于ds位置的元素互换位置
                        ui ds = degree_starts[degree[w]];
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

ui graph::_one_swapable_vertex(ui u, vector<ui>& I) {
#ifndef NDEBUG
    cout << "one swap " << u << endl;
#endif
    vector<ui> V;
    edge_t* p_edge = _nodes[u].edges;
    while (p_edge != NULL) {
#ifndef NDEBUG
        cout << _nodes[p_edge->node_id].node_id << " degree: " << _nodes[p_edge->node_id].degree
            << " node status: " << _nodes[p_edge->node_id].node_status
            << " counter: " << _nodes[p_edge->node_id].counter << endl;
#endif
        if (_nodes[p_edge->node_id].counter == 1 &&
                _nodes[p_edge->node_id].node_status == _NOMIS) {
            V.push_back(p_edge->node_id);
        }
        p_edge = p_edge->next_edge;
    }
#ifndef NDEBUG
    cout << V.size() << endl;
#endif
    if (V.size() <= 1) {
        I.swap(V);
        return I.size();
    }

    unordered_map<ui, ui> index;
    // ui* index = new ui[_n + 1];
    // memset(index, 0, sizeof(ui)*(_n + 1));
    for (ui i = 0; i < V.size(); ++ i) index[V[i]] = i + 1;

    // construct subgraph G[V]
    ui subgraph_n = V.size();
    graph subgraph(subgraph_n);
    for (ui i = 0; i < subgraph_n; ++ i)
        subgraph._add_node(i + 1, (ui)V[i]);
    for (ui i = 0; i < subgraph_n; ++ i) {
        edge_t* p_edge = _nodes[V[i]].edges;
        ui j = i + 1;
        while (p_edge != NULL && j < V.size()) {
            if (p_edge->node_id > V[i]) {
                if (p_edge->node_id > V[j]) j ++;
                else if (p_edge->node_id < V[j]) p_edge = p_edge->next_edge;
                else {
#ifndef NDEBUG
                    if (index[p_edge->node_id] == 0)
                        cout << "node " << p_edge->node_id << " is not in V\n";
#endif
                    subgraph._add_edge(i + 1, index[p_edge->node_id]);
                    j ++;
                    p_edge = p_edge->next_edge;
                }
            } else {
                p_edge = p_edge->next_edge;
            }
        }
    }
    // delete[] index;
    // subgraph.greedy_dynamic(I);
#ifndef NDEBUG
    subgraph.show();
#endif

    subgraph._greedy(I);
    return I.size();
}

ui graph::_two_swapable_vertex(ui u, ui& v, vector<ui>& I) {
#ifndef NDEBUG
    cout << "two swap " << u << ", " << v << endl;
#endif
    unordered_map<ui, vector<ui> > V;
    edge_t* p_edge = _nodes[u].edges;
    // cout << v << endl;
    while (p_edge != NULL) {
        if (_nodes[p_edge->node_id].counter == 2 &&
                _nodes[p_edge->node_id].node_status == _NOMIS) {
            ui w = 0;
            edge_t* p_edge_tmp = _nodes[p_edge->node_id].edges;
            while (p_edge_tmp != NULL) {
                if (_nodes[p_edge_tmp->node_id].node_status == _MIS &&
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
                V[w] = vector<ui>();
                V[w].push_back(p_edge->node_id);
            }
        }
        p_edge = p_edge->next_edge;
    }

    if (v == 0) {
        ui size = 0;
        for (auto it : V) {
            ui v_tmp = it.first;
            vector<ui> I_tmp;
            p_edge = _nodes[u].edges;
            while (p_edge != NULL) {
                if (_nodes[p_edge->node_id].counter == 1 &&
                        _nodes[p_edge->node_id].node_status == _NOMIS)
                    V[v_tmp].push_back(p_edge->node_id);
                p_edge = p_edge->next_edge;
            }
            p_edge = _nodes[v_tmp].edges;
            while (p_edge != NULL) {
                if (_nodes[p_edge->node_id].counter == 1 &&
                        _nodes[p_edge->node_id].node_status == _NOMIS)
                    V[v_tmp].push_back(p_edge->node_id);
                p_edge = p_edge->next_edge;
            }
            if (V[v_tmp].size() > 1) {
                sort(V[v_tmp].begin(), V[v_tmp].end());
                // ui* index = new ui[_n + 1];
                // memset(index, 0, sizeof(ui)*(_n + 1));
                unordered_map<ui, ui> index;
                for (ui i = 0; i < V[v_tmp].size(); ++ i) index[V[v_tmp][i]] = i + 1;

                ui subgraph_n = V[v_tmp].size();
                graph subgraph(subgraph_n);
                for (ui i = 0; i < subgraph_n; ++ i)
                    subgraph._add_node(i + 1, V[v_tmp][i]);

                for (ui i = 0; i < subgraph_n; ++ i) {
                    edge_t* p_edge = _nodes[V[v_tmp][i]].edges;
                    ui j = i + 1;
                    while (p_edge != NULL && j < subgraph_n) {
                        if (p_edge->node_id > V[v_tmp][i]) {
                            if (p_edge->node_id > V[v_tmp][j]) j ++;
                            else if (p_edge->node_id < V[v_tmp][j]) p_edge = p_edge->next_edge;
                            else {
#ifndef NDEBUG
                                if (index[p_edge->node_id] == 0)
                                    cout << "node " << p_edge->node_id << " is not in V.\n";
#endif
                                subgraph._add_edge(i + 1, index[p_edge->node_id]);
                                j ++;
                                p_edge = p_edge->next_edge;
                            }
                        } else {
                            p_edge = p_edge->next_edge;
                        }
                    }
                }

                // delete[] index;

                subgraph._greedy(I_tmp);
                v = I_tmp.size() > size ? v_tmp : v;
                size = max(size, (ui)I_tmp.size());
            }
        }
    } else {
        edge_t* p_edge = _nodes[u].edges;
        while (p_edge != NULL) {
            if (_nodes[p_edge->node_id].counter == 1 &&
                    _nodes[p_edge->node_id].node_status == _NOMIS)
                V[v].push_back(p_edge->node_id);
            p_edge = p_edge->next_edge;
        }
        p_edge = _nodes[v].edges;
        while (p_edge != NULL) {
            if (_nodes[p_edge->node_id].counter == 1 &&
                    _nodes[p_edge->node_id].node_status == _NOMIS)
                V[v].push_back(p_edge->node_id);
            p_edge = p_edge->next_edge;
        }
    }
#ifndef NDEBUG
    cout << V[v].size() << endl;
#endif

    if (v != 0 && V[v].size() > 1) {
        sort(V[v].begin(), V[v].end());
        unordered_map<ui, ui> index;
        // ui* index = new ui[_n + 1];
        // memset(index, 0, sizeof(ui)*(_n + 1));
        for (ui i = 0; i < V[v].size(); ++ i) index[V[v][i]] = i + 1;

        ui subgraph_n = V[v].size();
        graph subgraph(subgraph_n);
        for (ui i = 0; i < subgraph_n; ++ i)
            subgraph._add_node(i + 1, V[v][i]);

        for (ui i = 0; i < subgraph_n; ++ i) {
            edge_t* p_edge = _nodes[V[v][i]].edges;
            ui j = i + 1;
            while (p_edge != NULL && j < subgraph_n) {
                if (p_edge->node_id > V[v][i]) {
                    if (p_edge->node_id > V[v][j]) j ++;
                    else if (p_edge->node_id < V[v][j]) p_edge = p_edge->next_edge;
                    else {
#ifndef NDEBUG
                        if (index[p_edge->node_id] == 0)
                            cout << "node " << p_edge->node_id << " is not in V.\n";
#endif
                        subgraph._add_edge(i + 1, index[p_edge->node_id]);
                        j ++;
                        p_edge = p_edge->next_edge;
                    }
                } else {
                    p_edge = p_edge->next_edge;
                }
            }
        }
        // subgraph.show();

        // delete[] index;
        subgraph._greedy(I);
        // subgraph.greedy_dynamic(I);
    }

    return I.size();
}


/*
 * test for graph.h
 */
int main(int argc, char *argv[])
{
    // std::cout << argv[1] << std::endl;
    if (argc == 2) {
        string file_path = (string)argv[1] + ".bin";
        string mis_path = (string)argv[1] + ".mis";
        string inst_path = (string)argv[1] + ".inst";
        string inst_path2 = (string)argv[1] + ".1w.inst";
        graph g(file_path.c_str(), mis_path.c_str());
        g.read_graph();
        g.read_mis();
        g.experiment(inst_path.c_str());
        // g.experiment2(inst_path2.c_str());
    } else if (argc == 4) {
        graph g(argv[1], argv[2]);
        g.read_graph();
        g.read_mis();
        g.experiment(argv[3]);
        // g.experiment2(argv[3]);
    }
    else {
        cout << "error.\n";
    }
    return 0;
}
