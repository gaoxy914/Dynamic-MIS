#include "DyTwoSwap.h"

bool Node::_exist_edge(const int& id) {
    for (int w : edges) if (w == id) return true;
    return false;
}

void Node::_insert_edge(const int& id) {
    degree ++;
    edges.push_back(id);
}

void Node::_delete_edge(const int& id) {
    for (int i = 0; i < degree; ++ i) {
        if (edges[i] == id) {
            edges[i] = edges.back();
            edges.pop_back();
            break;
        }
    }
    degree --;
}

void Node::_insert_L(const int& id) {
    count ++;
    L[id] = 1;
}

void Node::_delete_L(const int& id) {
    count --;
    L.erase(id);
}

pair<int, int> Node::_get_pair() {
    int u = L.begin()->first;
    int v = (++L.begin())->first;
    return make_pair(min(u, v), max(u, v));
}

Graph::Graph() {
    n = m = mis = 0;
    utime = 0;
}

Graph::~Graph() {}

void Graph::load_graph(const char* path) {
    ifstream infile((_GRAPH_PATH + string(path)).c_str());
    if (!infile.is_open()) {
        cout << "graph file: " << path << " open failed." << endl;
        exit(1);
    }
    int u, v;
    infile >> n >> m;
    nodes.resize(n);
    for (int i = 0; i < n; ++ i) nodes[i].id = i;
    for (int i = 0; i < m; ++ i) {
        infile >> u >> v;
        nodes[u]._insert_edge(v);
        nodes[v]._insert_edge(u);
    }
    infile.close();
}

void Graph::load_mis(const char* path) {
    ifstream infile((_MIS_PATH + string(path)).c_str());
    if (!infile.is_open()) {
        cout << "mis file: " << path << " open failed." << endl;
        exit(1);
    }
    int id;
    infile >> mis;
    for (int i = 0; i < mis; ++ i) {
        infile >> id;
        nodes[id].status = 1;
        for (int u : nodes[id].edges) {
            nodes[u].status = 2;
            if (nodes[u].count == 0) nodes[id]._insert_L(u);
            else if (nodes[u].count == 1) {
                int v = nodes[u].L.begin()->first;
                nodes[v]._delete_L(u);
                _insert_L2(make_pair(min(v, id), max(v, id)), u);
            } else if (nodes[u].count == 2)
                _delete_L2(nodes[u]._get_pair(), u);
            nodes[u]._insert_L(id);
        }
    }
    for (int i = 0; i < n; ++ i) {
        if (nodes[i].status == 0 && nodes[i].edges.size() == 0) {
            nodes[i].status = 1;
            mis ++;
        }
    }
    infile.close();
}

void Graph::eff_exp(const char* path, const int& num) {
    ifstream infile ((_INST_PATH + string(path)).c_str());
    if (!infile.is_open()) {
        cout << "instruction file: " << path << " open failed." << endl;
        exit(1);
    }
    for (int i = 0; i < num; ++ i) {
        Update op;
        infile >> op.type;
        if (op.type == 0) {
            infile >> op.u >> op.v;
            _insert_edge(op.u, op.v);
        } else if (op.type == 1) {
            infile >> op.u >> op.v;
            _delete_edge(op.u, op.v);
        } else if (op.type == 2) {
            infile >> op.u;
            _delete_vertex(op.u);
        }
    }
    cout << "DyTwoSwap\t" << string(path) << "\tutime = " << utime << "\t|V| = " << n << "\t|I| = " << mis << endl;
    infile.close();
}

void Graph::check() {
    int check_mis = 0;
    bool maximal = true;
    for (auto node : nodes) {
        if (node.status == 1) {
            int check_count = 0;
            check_mis ++;
            for (int w : node.edges) {
                if (nodes[w].status == 1) {
                    cout << "WA: two adjacent vertices in I.\n";
                    exit(1);
                } else if (nodes[w].status == 2 && nodes[w].count == 1) {
                    check_count ++;
                }
            }
            if (check_count != node.count) {
                cout << "WA: " << node.id << '\t' << node.status << " wrong count. " << check_count << '\t' << node.count << endl;
                exit(1);
            }
            for (auto w : node.L) {
                if (nodes[w.first].count != 1 || nodes[w.first].status == 1) {
                    cout << "WA: " << node.id << " wrong L list. " << w.first << '\t' << nodes[w.first].status << '\t' << nodes[w.first].count << endl;
                    exit(1);
                }
                for (auto n : node.L) {
                    if (n.first != w.first && !nodes[w.first]._exist_edge(n.first)) {
                        cout << "WA: " << node.id << " not 1-maximal.\t" << n.first << '\t' << w.first << endl;
                        exit(1);
                    }
                }
            }
        } else if (node.status == 2) {
            int check_count = 0;
            bool found = false;
            for (int w : node.edges) {
                if (nodes[w].status == 1) {
                    found = true;
                    check_count ++;
                }
            }
            if (!found) {
                cout << "WA: not maximal. " << node.id << '\t' << node.status << endl;
                exit(1);
            }
            if (check_count != node.count) {
                cout << "WA: " << node.id << '\t' << node.status << " wrong count. " << check_count << '\t' << node.count << endl;
                cout << "L :\n";
                for (auto iter : node.L) cout << iter.first << endl;
                cout << "N :\n";
                for (int w : node.edges) {
                    if (nodes[w].status == 1) {
                        cout << w << endl;
                    }
                }
                exit(1);
            }
        } else if (node.status == 0) {
            cout << "WA: not maximal. " << node.id << '\t' << node.status << endl;
            exit(1);
        }
    }
    for (auto iter : L2) {
        for (auto iter2 : iter.second) {
            if (nodes[iter2.first].status != 2 || nodes[iter2.first].count != 2 || nodes[iter2.first]._get_pair() != iter.first) {
                cout << "WA: (" << iter.first.first << "," << iter.first.second << ")\t" << " wrong L2.\n";
                cout << iter2.first << '\t' << nodes[iter2.first].status << endl;
                for (auto iter3 : nodes[iter2.first].L) cout << iter3.first << endl;
                exit(1);
            }
        }
    }
    if (check_mis != mis) {
        cout << "WA: wrong mis counter.\n";
        exit(1);
    }
}

void Graph::_insert_L2(const pair<int, int>& p, const int& v) {
    L2[p].insert(make_pair(v, 1));
}

void Graph::_delete_L2(const pair<int, int>& p, const int& v) {
    L2[p].erase(v);
    if (L2[p].size() == 0) L2.erase(p);
}

void Graph::_insert_candidate1(const int& u, const int& v) {
    if (candidates1.count(u) == 0) {
        vector<int> C; C.push_back(v); candidates1[u] = C;
    } else candidates1[u].push_back(v);
}

void Graph::_insert_candidate2(const pair<int, int>& p, const int& v) {
    if (candidates2.count(p) == 0) {
        vector<int> C; C.push_back(v); candidates2[p] = C;
    } else candidates2[p].push_back(v);
}

void Graph::_find_candidate(const int& id) {
    for (int u : nodes[id].edges) {
        if (nodes[u].count == 1 && nodes[u].status == 2) {
            int v = nodes[u].L.begin()->first;
            if (candidates1.count(v) == 0) {
                vector<int> C; C.push_back(u); candidates1[v] = C;
            } else candidates1[v].push_back(u);
        } else if (nodes[u].count == 2 && nodes[u].status == 2) {
            pair<int, int> p = nodes[u]._get_pair();
            if (candidates2.count(p) == 0) {
                vector<int> C; C.push_back(u); candidates2[p] = C;
            } else candidates2[p].push_back(u);
        }   
    }
}

void Graph::_delete_vertex(const int& id) {
#ifdef _LINUX_
    struct timeval start, end;
    gettimeofday(&start, NULL);
#endif
    n --;
    for (int u : nodes[id].edges) nodes[u]._delete_edge(id);
    if (nodes[id].status == 1) {
        int u = id;
        if (nodes[id].count > 0) u = nodes[id].L.begin()->first;
        _move_out(id);
        if (u != id) _move_in(u);
        _find_candidate(id);
        if (!candidates1.empty() || !candidates2.empty()) _extend();
    } else if (nodes[id].status == 2) {
        if (nodes[id].count == 1)
            nodes[nodes[id].L.begin()->first]._delete_L(id);
        else if (nodes[id].count == 2)
            _delete_L2(nodes[id]._get_pair(), id);
    }
    nodes[id].status = -1;
    nodes[id].edges.clear();
    nodes[id].L.clear();
#ifdef _LINUX_
    gettimeofday(&end, NULL);
    long long mtime, seconds, useconds;
    seconds = end.tv_sec - start.tv_sec;
    useconds = end.tv_usec - start.tv_usec;
    mtime = seconds*1000000 + useconds;
    utime += mtime;
#endif
}

void Graph::_insert_edge(int u, int v) {
#ifdef _LINUX_
    struct timeval start, end;
    gettimeofday(&start, NULL);
#endif
    m ++;
    nodes[u]._insert_edge(v);
    nodes[v]._insert_edge(u);
    if (nodes[u].status == 1 && nodes[v].status == 1) {
        if (nodes[u].count == 0 && nodes[v].count == 0) {
            if (nodes[u].degree < nodes[v].degree) swap(u, v);
            _move_out(u);
            nodes[u]._insert_L(v);
            nodes[v]._insert_L(u);
        } else {
            if (nodes[u].count == 0) swap(u, v);
            int w = nodes[u].L.begin()->first;
            _move_out(u);
            nodes[u]._insert_L(v);
            nodes[v]._insert_L(u);
            _move_in(w);
        }
        _find_candidate(u);
        if (nodes[u].count == 1) _insert_candidate1(v, u);
        else if (nodes[u].count == 2) _insert_candidate2(nodes[u]._get_pair(), u);
        if (!candidates1.empty() || !candidates2.empty()) _extend();
    } else if (nodes[u].status != nodes[v].status) {
        if (nodes[v].status == 1) swap(u, v);
        if (nodes[v].count == 1) {
            int w = nodes[v].L.begin()->first;
            nodes[w]._delete_L(v);
            _insert_L2(make_pair(min(u, w), max(u, w)), v);
        } else if (nodes[v].count == 2) _delete_L2(nodes[v]._get_pair(), v);
        nodes[v]._insert_L(u);
    }
#ifdef _LINUX_
    gettimeofday(&end, NULL);
    long long mtime, seconds, useconds;
    seconds = end.tv_sec - start.tv_sec;
    useconds = end.tv_usec - start.tv_usec;
    mtime = seconds*1000000 + useconds;
    utime += mtime;
#endif
}

void Graph::_delete_edge(int u, int v) {
#ifdef _LINUX_
    struct timeval start, end;
    gettimeofday(&start, NULL);
#endif
    m --;
    nodes[u]._delete_edge(v);
    nodes[v]._delete_edge(u);
    if (nodes[u].status == 2 && nodes[v].status == 2) {
        if (nodes[u].count == 1 && nodes[v].count == 1) {
            int x = nodes[u].L.begin()->first, y = nodes[v].L.begin()->first;
            if (x > y) swap(x, y);
            if (x == y) {
                _move_out(x);
                _move_in(u);
                _move_in(v);
                _find_candidate(x);
                if (!candidates1.empty() || !candidates2.empty()) _extend();
            } else if (L2.count(make_pair(x, y)) != 0) {
                unordered_map<int, int> N;
                for (int w : nodes[x].edges) N[w] = 1;
                for (int w : nodes[y].edges) N[w] = 1;
                for (auto iter : L2[make_pair(x, y)]) {
                    if (N.count(iter.first) == 0) {
                        _move_out(x); _move_out(y);
                        _move_in(u); _move_in(v); _move_in(iter.first);
                        _find_candidate(x); _find_candidate(y);
                        if (!candidates1.empty() || !candidates2.empty()) _extend();
                    }
                }
            }
        } else if (max(nodes[u].count, nodes[v].count) == 2) {
            if (nodes[v].count == 2) swap(u, v);
            pair<int, int> p = nodes[u]._get_pair();
            if ((nodes[v].count == 2 && nodes[v]._get_pair() == p) ||
                (nodes[v].count == 1 && (nodes[v].L.begin()->first == p.first || nodes[v].L.begin()->first == p.second))) {
                _insert_candidate2(nodes[u]._get_pair(), u);
                _extend();
            }

        }
    } else if (min(nodes[u].status, nodes[v].status) == 1) {
        if (nodes[v].status == 1) swap(u, v);
        nodes[v]._delete_L(u);
        if (nodes[v].count == 0) {
            nodes[u]._delete_L(v);
            _move_in(v);
        } else if (nodes[v].count == 1) {
            int w = nodes[v].L.begin()->first;
            nodes[w]._insert_L(v);
            _delete_L2(make_pair(min(u, w), max(u, w)), v);
            _insert_candidate1(w, v);
            _extend();
        } else if (nodes[v].count == 2) {
            pair<int, int> p = nodes[v]._get_pair();
            _insert_L2(p, v);
            _insert_candidate2(p, v);
            _extend();
        }
    }

#ifdef _LINUX_
    gettimeofday(&end, NULL);
    long long mtime, seconds, useconds;
    seconds = end.tv_sec - start.tv_sec;
    useconds = end.tv_usec - start.tv_usec;
    mtime = seconds*1000000 + useconds;
    utime += mtime;
#endif
}

void Graph::_move_in(const int& id) {
    mis ++;
    nodes[id].status = 1;
    nodes[id].L.clear();
    nodes[id].count = 0;
    for (int w : nodes[id].edges) {
        if (nodes[w].status == 2) {
            if (nodes[w].count == 0) nodes[id]._insert_L(w);
            else if (nodes[w].count == 1) {
                int v = nodes[w].L.begin()->first;
                nodes[v]._delete_L(w);
                _insert_L2(make_pair(min(v, id), max(v, id)), w);
            } else if (nodes[w].count == 2) _delete_L2(nodes[w]._get_pair(), w);
            nodes[w]._insert_L(id);
        }
    }
}

void Graph::_move_out(const int& id) {
    mis --;
    nodes[id].status = 2;
    nodes[id].L.clear();
    nodes[id].count = 0;
    for (int w : nodes[id].edges) {
        if (nodes[w].status == 2) {
            nodes[w]._delete_L(id);
            if (nodes[w].count == 1) {
                int v = nodes[w].L.begin()->first;
                nodes[v]._insert_L(w);
                _delete_L2(make_pair(min(v, id), max(v, id)), w);
            } else if (nodes[w].count == 2)
                _insert_L2(nodes[w]._get_pair(), w);
        }
    }
}

void Graph::_extend() {
    while (!candidates1.empty() || !candidates2.empty()) {
        if (!candidates1.empty()) _oneswap();
        else if (!candidates2.empty()) _twoswap();
    }
}

void Graph::_oneswap() {
    int v = candidates1.begin()->first;
    vector<int> newL = candidates1.begin()->second;
    candidates1.erase(v);
    if (nodes[v].status == 1) {
        for (int u : newL) {
            if (nodes[u].status == 2 && nodes[u].count == 1 && nodes[u].L.begin()->first == v) {
                int intersection = 0;
                for (int w : nodes[u].edges)
                    if (nodes[v].L.count(w) != 0) intersection ++;
                if (intersection + 1 < nodes[v].count) {
                    _move_out(v);
                    _move_in(u);
                    for (int w : nodes[v].edges)
                        if (nodes[w].count == 0 && nodes[w].status == 2) _move_in(w);
                    _find_candidate(v);
                    return;
                }
            }
        }
#ifdef _PERTURBATION_
        if (nodes[v].status == 1 && nodes[v].count > 0 ) {
            int u = v, min_degree = nodes[v].edges.size();
            for (int w : nodes[v].edges) {
                if (nodes[w].count == 1 && nodes[w].edges.size() < min_degree) {
                    u = w;
                    min_degree = nodes[w].edges.size();
                }
            }
            if (u != v) {
                _move_out(v);
                _move_in(u);
                for (int w : nodes[v].edges)
                    if (nodes[w].count == 0 && nodes[w].status == 2) _move_in(w);
                _find_candidate(v);
                return;
            }
        }
#endif
        unordered_map<int, bool> N;    
        for (int u : newL) for (int w : nodes[u].edges) N[w] = 1;
        for (int w : nodes[v].edges)
            if (nodes[w].count == 2 && N.count(w) == 0)
                _insert_candidate2(nodes[w]._get_pair(), w);
    }
}

void Graph::_twoswap() {
    pair<int, int> p = candidates2.begin()->first;
    vector<int> newL2 = candidates2.begin()->second;
    candidates2.erase(p);
    if (nodes[p.first].status == 1 && nodes[p.second].status == 1) {
        unordered_map<int, bool> Z; 
        int numZ = 0;
        vector<int> Y; 
        for (auto iter : L2[p]) { Z[iter.first] = 1; numZ ++; Y.push_back(iter.first); }
        for (auto iter : nodes[p.second].L) { Z[iter.first] = 1; numZ ++; }
        for (auto iter : nodes[p.first].L) Y.push_back(iter.first);
        while (!newL2.empty()) {
            int x = newL2.back();
            newL2.pop_back();
            if (nodes[x].status == 2 && nodes[x].count == 2 && nodes[x]._get_pair() == p) {
                int numZ_valid = numZ - 1;
                unordered_map<int, bool> N; 
                for (int w : nodes[x].edges) { N[w] = Z.count(w) == 0 ? 0 : 1; numZ_valid -= N[w]; }
                for (int y : Y) {
                    if (N.count(y) == 0 && y != x) {
                        int intersection = 0;
                        if (Z.count(y) != 0) intersection ++;
                        for (int w : nodes[y].edges) if (Z.count(w) != 0 && N.count(w) == 0) intersection ++;
                         if (intersection < numZ_valid) {
                            _move_out(p.first); _move_out(p.second);
                            _move_in(x); _move_in(y);
                            for (int w : nodes[p.first].edges)
                                if (nodes[w].status == 2 && nodes[w].count == 0) _move_in(w);
                            for (int w : nodes[p.second].edges)
                                if (nodes[w].status == 2 && nodes[w].count == 0) _move_in(w);
                            unordered_map<int, bool> I;
                            for (int w : nodes[p.first].edges) {
                                if (nodes[w].count == 1 && nodes[w].status == 2)
                                    _insert_candidate1(nodes[w].L.begin()->first, w);
                                else if (nodes[w].count == 2 && nodes[w].status == 2)
                                    _insert_candidate2(nodes[w]._get_pair(), w);
                                I[w] = true;
                            }
                            for (int w : nodes[p.second].edges) {
                                if (I.count(w) == 0) {
                                    if (nodes[w].count == 1 && nodes[w].status == 2)
                                        _insert_candidate1(nodes[w].L.begin()->first, w);
                                    else if (nodes[w].count == 2 && nodes[w].status == 2)
                                        _insert_candidate2(nodes[w]._get_pair(), w);
                                }
                            }
                            return;
                         }
                    }
                }
            }
        }
#ifdef _PERTURBATION_
        if (nodes[p.first].status == 1 && nodes[p.first].count > 0) {
            int u = p.first, min_degree = nodes[p.first].edges.size();
            for (int w : nodes[p.first].edges) {
                if (nodes[w].count == 1 && nodes[w].edges.size() < min_degree) {
                    u = w;
                    min_degree = nodes[w].edges.size();
                }
            }
            if (u != p.first) {
                _move_out(p.first);
                _move_in(u);
                for (int w : nodes[p.first].edges)
                    if (nodes[w].count == 0 && nodes[w].status == 2) _move_in(w);
                _find_candidate(p.first);  
            }
        }
        if (nodes[p.second].status == 1 && nodes[p.second].count > 0) {
            int u = p.second, min_degree = nodes[p.second].edges.size();
            for (int w : nodes[p.second].edges) {
                if (nodes[w].count == 1 && nodes[w].edges.size() < min_degree) {
                    u = w;
                    min_degree = nodes[w].edges.size();
                }
            }
            if (u != p.second) {
                _move_out(p.second);
                _move_in(u);
                for (int w : nodes[p.second].edges)
                    if (nodes[w].count == 0 && nodes[w].status == 2) _move_in(w);
                _find_candidate(p.second);  
            }
        }
#endif
    }
}

int main(int argc, char const *argv[])
{
    string id = string(argv[1]);
    string graph_path = id + ".graph";
    string mis_path = id + ".mis";
    string inst_path = id + ".inst";
    Graph g;
    g.load_graph(graph_path.c_str());
    g.load_mis(mis_path.c_str());
    g.check();
    g.eff_exp(inst_path.c_str(), atoi(argv[2]));
    g.check();
    return 0;
}
