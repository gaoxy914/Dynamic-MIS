#include "graph.h"

/* FILE* open_file(const char* file_path, const char* mode){
    FILE* file = fopen(file_path, mode);
    if (file == NULL) {
        cout << "file: " << file_path << " open failed." << endl;
        exit(1);
    }
    return file;
} */

// graph::graph(const char* _file_path, const char * _mis_path) {
graph::graph(const char* _file_path) {
    file_path = string(_file_path);
    // mis_path = string(_mis_path);
    n = m = mis = 0;
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
    ifstream infile(file_path.c_str());
    if (!infile.is_open()) {
        cout << "file: " << file_path << " open failed." << endl;
        exit(1);
    }

    infile >> n >> m;
    cout << "n = " << n << "; m = " << m << endl;

    if (nodes == NULL) nodes = new node[n + 1];
    for (int i = 1; i <= n; ++i) {
        nodes[i] = node((unsigned int)i);
    }

    unsigned int u, v;
    for (int i = 0; i < m; ++i) {
        infile >> u >> v;
        add_edge(u, v);
    }
    infile.close();
}

/*
 * mis file format
 * mis
 * u // u in IS
 */
void graph::read_mis() {
    ifstream infile(mis_path.c_str());
    if (!infile.is_open()) {
        cout << "file: " << mis_path << " open failed." << endl;
        exit(1);
    }
    infile >> mis;
    cout << "mis = " << mis << endl;
    unsigned int u;
    for (int i = 0; i < mis; ++i) {
        infile >> u;
        nodes[u].node_status = _MIS;
        nodes[u].counter = 0;
        edge* p_edge = nodes[u].edges;
        while (p_edge != NULL) {
            if (nodes[p_edge->node_id].node_status == _MIS) {
                cout << "WA two adjacent vertices in is.\n";
                exit(1);
            } else {
                nodes[p_edge->node_id].node_status = _ADJACENT;
                nodes[p_edge->node_id].counter++;
            }
            p_edge = p_edge->next_edge;
        }
    }
    infile.close();
}

void graph::handle_update(const update& _update) {
    switch(_update.type) {
    /* case _VERTEX_ADDITION:
        add_vertex(_update.u);
        break;
    case _VERTEX_DELETION:
        delete_vertex(_update.u);
        break; */
    case _EDGE_ADDITION:
        add_edge(_update.u, _update.v);
        break;
    case _EDGE_DELETION:
        delete_edge(_update.u, _update.v);
        break;
    default:
        break;
    }
}

void graph::greedy() {
    unsigned int* degree_buckets = new unsigned int[n];
    memset(degree_buckets, 0, sizeof(unsigned int)*n);
    for (int i = 1; i <= n; ++i)
        degree_buckets[nodes[i].degree]++;

    for (int i = 1; i < n; ++i)
        degree_buckets[i] += degree_buckets[i - 1];

    unsigned int* order = new unsigned int[n + 1]; // order[i]  = j means that the order of node i is j
    for (int i = 1; i <= n; ++i) {
        order[i] = degree_buckets[nodes[i].degree];
        degree_buckets[nodes[i].degree]--;
    }

    /* for (int i = 1; i <= n; ++i)
        std::cout << "vertex " << i << " order " << order[i] << std::endl; */
    unsigned int* greedy_order = new unsigned int[n + 1]; // greedy_order[i] = j means that No.i is node j
    for (int i = 1; i <= n; ++i) {
        greedy_order[order[i]] = i;
    }

    /* for (int i = 1; i <= n; ++i)
        std::cout << "order " << i << " vertex " << greedy_order[i] << std::endl;*/

    for (int i = 1; i <= n; ++i) {
        if (nodes[greedy_order[i]].node_status == _UNVISITD) {
            nodes[greedy_order[i]].node_status = _MIS;
            mis++;
            edge* p_edge = nodes[greedy_order[i]].edges;
            while (p_edge != NULL) {
                nodes[p_edge->node_id].node_status = _ADJACENT;
                nodes[p_edge->node_id].counter++;
                p_edge = p_edge->next_edge;
            }
        }
    }

    cout << "Greedy MIS: " << mis << endl;

    delete[] degree_buckets;
    delete[] order;
    delete[] greedy_order;

    check_mis();
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
        if (nodes[u].node_status != _UNVISITD) continue;
        nodes[u].node_status = _MIS;
        res ++;
        I.push_back(nodes[u].node_id);
        edge* p_edge = nodes[u].edges;
        while (p_edge != NULL) {
            unsigned int v = p_edge->node_id;
            // nodes[v].counter++;
            if (nodes[v].node_status == _UNVISITD) {
                nodes[v].node_status = _ADJACENT;
                edge* p_edge_v = nodes[v].edges;
                while (p_edge_v != NULL) {
                    unsigned int w = p_edge_v->node_id;
                    if (nodes[w].node_status == _UNVISITD && w != u) {
                        // 把w移动到和它度一样元素的首位，即和位于ds位置的元素互换位置
                        unsigned ds = degree_starts[degree[w]];
                        order[no[ds]] = order[w];
                        swap(no[ds], no[order[w]]);
                        order[w] = ds;
                        degree_starts[degree[w]]++;
                        degree[w]--;
                        // 如果再向前交换，交换的位置不能小于i + 1，否则改点不会被处理
                        if (degree_starts[degree[w]] <= i) degree_starts[degree[w]] = i + 1;
                    }
                    p_edge_v = p_edge_v->next_edge;
                }
            }
            p_edge = p_edge->next_edge;
        }
    }

    cout << "MIS number: " << res << endl;

    for (int i = 1; i <= n; ++i) {
        if (nodes[i].node_status == _MIS)
            cout << nodes[i].node_id << " ";
    }
    cout << endl;

    delete[] degree;
    delete[] degree_buckets;
    delete[] order;
    delete[] no;
    delete[] degree_starts;
}


void graph::show() {
    for (int i = 1; i <= n; ++i) {
        cout << nodes[i].node_id << " degree: " << nodes[i].degree << " adjacent list : ";
        edge* p_edge = nodes[i].edges;
        while (p_edge != NULL) {
            cout << p_edge->node_id << ", ";
            p_edge = p_edge->next_edge;
        }
        cout << endl;
    }
}

void graph::experiment(const char* _inst_file) {
    vector<update> updates;
    ifstream infile(_inst_file);
    if (!infile.is_open()) {
        cout << "instruction file: " << file_path << " open failed." << endl;
        exit(1);
    }

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
        handle_update(updates[i]);
    }

}

void graph::check_mis() {
    bool maximal = true;
    // cout << "in check mis.\n";
    for (int i = 1; i <= n; ++i) {
        // std::cout << i << std::endl;
        if (nodes[i].node_status == _MIS) {
            edge* p_edge = nodes[i].edges;
            while (p_edge != NULL) {
                if (nodes[p_edge->node_id].node_status == _MIS)
                    cout << "WA two adjacent vertices in is.\n";
                p_edge = p_edge->next_edge;
            }
        } else if (nodes[i].node_status == _ADJACENT && maximal) {
            bool find = false;
            edge* p_edge = nodes[i].edges;
            while (p_edge != NULL) {
                if (nodes[p_edge->node_id].node_status == _MIS) {
                    find = true;
                    break;
                }
                p_edge = p_edge->next_edge;
            }
            if (!find) {
                maximal = false;
                cout << "WA not maximal.\n";
            }
        }
    }
}

void graph::add_node(unsigned int index, unsigned int node_id) {
    if (index <= 0 || index > n) {
        cout << "array out of range.\n";
        exit(1);
    }
    nodes[index] = node(node_id);
}

/* void graph::add_vertex(const unsigned int& u) {
    if (u <= 0 || u > n + _INST_NUM) {
        cout << "vertex number " << u << " our of range.\n";
        return;
    }
}

void graph::delete_vertex(const unsigned int& u) {
    if (u <= 0 || u > n || nodes[u].node_status == _DELETED) {
        cout << "vertex " << u << " does not exist.\n";
        return;
    }
    edge* p_edge = nodes[u].edges;
    while (p_edge != NULL) {
        delete_edge(u, p_edge->node_id);
        p_edge = p_edge->next_edge;
    }
    nodes[u].node_status = _DELETED;
} */

void graph::add_edge(unsigned int u, unsigned int v) {
    if (u <= 0 || u > n || v <= 0 || v > n) {
        cout << "vertices" << u << " or " << v << "does not exist.\n";
        return;
    }
    if (nodes[u].node_status == _DELETED || nodes[v].node_status == _DELETED) {
        cout << "vertices" << u << " or " << v << "is deleted.\n";
        return;
    }

    nodes[u].degree++;
    edge* p_newedge = new edge(v);
    if (nodes[u].edges == NULL) nodes[u].edges = p_newedge;
    else {
        edge* p_edge = nodes[u].edges;
        edge* p_pre_edge = p_edge;
        while (p_edge != NULL && p_edge->node_id < v) {
            p_pre_edge = p_edge;
            p_edge = p_edge->next_edge;
        }
        p_pre_edge->next_edge = p_newedge;
        p_newedge->next_edge = p_edge;
    }


    nodes[v].degree++;
    p_newedge = new edge(u);
    if (nodes[v].edges == NULL) nodes[v].edges = p_newedge;
    else {
        edge* p_edge = nodes[v].edges;
        edge* p_pre_edge = p_edge;
        while (p_edge != NULL && p_edge->node_id < u) {
            p_pre_edge = p_edge;
            p_edge = p_edge->next_edge;
        }
        p_pre_edge->next_edge = p_newedge;
        p_newedge->next_edge = p_edge;
    }

    /* if (nodes[u].node_status == _MIS && nodes[v].node_status == _MIS) {
        if (nodes[u].degree < nodes[v].degree) swap(u, v);
        vector<unsigned int> v_in, v_out;
        // localserach_one_imp(u, v_in, v_out);
        if (v_in.size() > v_out.size()) {
            // update_neighbors(u);
            update_neighbors(v_in, v_out);
            return;
        }

        v_in.clear();
        v_out.clear();
        // localserach_one_imp(u, v_in, v_out);
        if (v_in.size() > v_out.size()) {
            // update_neighbors(v);
            update_neighbors(v_in, v_out);
            return;
        }

        update_neighbors(u);

    } else if (nodes[u].node_status == _MIS && nodes[v].node_status == _ADJACENT)
        nodes[v].counter++;
    else if (nodes[u].node_status == _ADJACENT && nodes[v].node_status == _MIS)
        nodes[u].counter++; */
}

void graph::delete_edge(unsigned int u, unsigned int v) {
    if (u <= 0 || u > n || v <= 0 || v > n) {
        cout << "vertices" << u << " or " << v << "does not exist.\n";
        return;
    }
    if (nodes[u].node_status == _DELETED || nodes[v].node_status == _DELETED) {
        cout << "edge (" << u << ", " << v << ") does not exits.\n";
        return;
    }
    edge* p_edge = nodes[u].edges;
    edge* p_pre_edge = p_edge;
    while (p_edge != NULL) {
        if (p_edge->node_id == v) {
            if (p_pre_edge == p_edge)
                nodes[u].edges = p_edge->next_edge;
            else
                p_pre_edge->next_edge = p_edge->next_edge;
            delete p_edge;
            nodes[u].degree--;
            // cout << "delete edge: (" << u << ", " << v << ")\n";
            break;
        }
        p_pre_edge = p_edge;
        p_edge = p_edge->next_edge;
    }
    if (p_edge == NULL) {
        cout << "edge (" << u << ", " << v << ") does not exist.\n";
        return;
    }

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

    if (p_edge == NULL) {
        cout << "edge (" << u << ", " << v << ") does not exist.\n";
        return;
    }

    /* if (nodes[u].node_status == _ADJACENT && nodes[v].node_status == _MIS)
        swap(u, v);
    if (nodes[u].node_status == _MIS && nodes[v].node_status == _ADJACENT) {
        nodes[v].counter--;
        if (nodes[v].counter == 0) {
            nodes[v].node_status = _MIS;
            mis++;
            p_edge_v = nodes[v].edges;
            while (p_edge_v != NULL) {
                nodes[p_edge_v->node_id].counter++;
                p_edge_v = p_edge_v->next_edge;
            }
        }
    } */
}

void graph::update_inf(const update& _update) {
    unsigned int u, v;
    switch(_update.type) {
    case _VERTEX_ADDITION:
        // add_vertex(_update.u);
        break;
    case _VERTEX_DELETION:
        // delete_vertex(_update.u);
        break;
    case _EDGE_ADDITION:
        u = _update.u; v = _update.v;
        if (nodes[u].node_status == _MIS && nodes[v].node_status == _MIS) {
            nodes[u].node_status = _CONFLICT;
            nodes[v].node_status = _CONFLICT;
        } else if (nodes[u].node_status == _MIS && nodes[v].node_status == _ADJACENT) {
            nodes[v].counter++;
        } else if (nodes[u].node_status == _ADJACENT && nodes[v].node_status == _MIS) {
            nodes[u].counter++;
        }

        break;
    case _EDGE_DELETION:
        u = _update.u; v = _update.v;
        if (nodes[u].node_status == _ADJACENT && nodes[v].node_status == _MIS)
            swap(u, v);
        if (nodes[u].node_status == _MIS && nodes[v].node_status == _ADJACENT) {
            nodes[v].counter--;
            if (nodes[v].counter == 0)
                nodes[v].node_status = _UNVISITD;
        }
        break;
    default:
        break;
    }
}

int graph::one_improvement_vertex(unsigned int u, vector<unsigned int>& I) {
    vector<unsigned int> V;
    edge* p_edge = nodes[u].edges;
    while (p_edge != NULL) {
        if (nodes[p_edge->node_id].counter == 1) V.push_back(p_edge->node_id);
        p_edge = p_edge->next_edge;
    }
    unsigned int* index = new unsigned int[n + 1];
    memset(index, 0, sizeof(unsigned int)*(n + 1));
    for (int i = 0; i < V.size(); ++ i) index[V[i]] = i + 1;

    // construct subgraph G[V]
    cout << "subgraph initialize.\n";
    unsigned int subgraph_n = V.size();
    graph subgraph(subgraph_n);
    for (int i = 0; i < subgraph_n; ++ i)
        subgraph.add_node(i + 1, (unsigned int)V[i]);
    for (int i = 0; i < subgraph_n; ++ i) {
        edge* p_edge = nodes[V[i]].edges;
        while (p_edge != NULL) {
            if (p_edge->node_id > V[i] && find(V.begin(), V.end(), p_edge->node_id) != V.end())
                subgraph.add_edge(i + 1, index[p_edge->node_id]);
        }
    }
    // show subgraph
    cout << "subgraph complete.\n";
    subgraph.show();

    delete[] index;

    subgraph.greedy_dynamic(I);

    return I.size();
}

int graph::two_improvement_vertex(unsigned int u, unsigned int v, vector<unsigned int>& I) {
    return 0;
}

void graph::test_subgraph() {
    vector<unsigned int> V;
    vector<unsigned int> I;
    for (int i = 1; i <= n; ++i) {
        if (rand()/(double)RAND_MAX > .5) {
            V.push_back(i);
            cout << i << " ";
        }
    }
    cout << V.size() << endl;
    unsigned int n_subgraph = V.size();
    unsigned int* index = new unsigned int[n + 1];
    memset(index, 0, sizeof(unsigned int)*(n + 1));
    for (int i = 0; i < V.size(); ++ i) {
        index[V[i]] = i + 1;
    }

    // implement with pointer

    /* node* subgraph = new node[n_subgraph + 1];
    cout << "subgraph initialize.\n";
    for (int i = 0; i < V.size(); ++i) {
        subgraph[i + 1] = node((unsigned int)V[i]);
        edge* p_edge = nodes[V[i]].edges;
        while (p_edge != NULL) {
            if (find(V.begin(), V.end(), p_edge->node_id) != V.end()) {
                subgraph[i + 1].degree++;
                edge* p_newedge = new edge(index[p_edge->node_id]);
                p_newedge->next_edge = subgraph[i + 1].edges;
                subgraph[i + 1].edges = p_newedge;
            }
            p_edge = p_edge->next_edge;
        }
    }

    for (int i = 1; i <= n_subgraph; ++i) {
        cout << subgraph[i].node_id << " degree: " << subgraph[i].degree
            << " node status " << subgraph[i].node_status << " adjacent list : ";
        edge* p_edge = subgraph[i].edges;
        while (p_edge != NULL) {
            cout << p_edge->node_id << ", ";
            p_edge = p_edge->next_edge;
        }
        cout << endl;
    }

    delete[] subgraph; */

    // implement by class

    graph subgraph(n_subgraph);
    for (int i = 0; i < n_subgraph; ++ i) {
        // cout << i << " " << V[i] << endl;
        subgraph.add_node(i + 1, (unsigned int)V[i]);
    }
    for (int i = 0; i < n_subgraph; ++ i) {
        edge* p_edge = nodes[V[i]].edges;
        /* while (p_edge != NULL) {
            if (p_edge->node_id > V[i]) {
                if (find(V.begin(), V.end(), p_edge->node_id) != V.end()) {
#ifndef NDEBUG
                    if (index[p_edge->node_id] == 0)
                        cout << "node " << p_edge->node_id << " is not in V\n";
#endif
                    subgraph.add_edge(i + 1, index[p_edge->node_id]);
                }
            }
            p_edge = p_edge->next_edge;
        } */
        int j = i + 1;
        while (p_edge != NULL && j < V.size()) {
            if (p_edge->node_id > V[i]) {
                if (p_edge->node_id > V[j]) j ++;
                else if (p_edge->node_id < V[j]) p_edge = p_edge->next_edge;
                else {
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

    subgraph.show();

    subgraph.greedy_dynamic(I);

    for (auto i : I) cout << i << " ";
    cout << endl;

}

void graph::test() {
    delete_edge(3, 5);
    delete_edge(4, 2);
}

/*
 * test for graph.h
 */
int main(int argc, char *argv[])
{
    // std::cout << argv[1] << std::endl;
    graph g(argv[1]);
    g.read_graph();
    g.show();
    // g.test();
    // g.show();
    // g.greedy();
    // g.greedy_dynamic();
    g.test_subgraph();
    return 0;
}
