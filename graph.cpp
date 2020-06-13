#include "graph.h"

/* FILE* open_file(const char* file_path, const char* mode){
    FILE* file = fopen(file_path, mode);
    if (file == NULL) {
        cout << "file: " << file_path << " open failed." << endl;
        exit(1);
    }
    return file;
} */

graph::graph(const char* _file_path) {
    file_path = string(_file_path);
    n = m = mis = 0;
    nodes = NULL;
    // edges = NULL;
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

void graph::read_graph(){
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
        // nodes[i].node_status = _MIS;
    }
    // mis = n;

    unsigned int u, v;
    for (int i = 0; i < m; ++i) {
        infile >> u >> v;
        add_edge(u, v);
    }
    infile.close();

    // cout << "MIS number: " << mis << endl;
    // check_mis();
}

void graph::handle_update(const update& _update) {
    switch(_update.type) {
    case _VERTEX_ADDITION:
        add_vertex(_update.u);
        break;
    case _VERTEX_DELETION:
        delete_vertex(_update.u);
        break;
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

    unsigned int* order = new unsigned int[n + 1];
    for (int i = 1; i <= n; ++i) {
        order[i] = degree_buckets[nodes[i].degree];
        degree_buckets[nodes[i].degree]--;
    }

    /* for (int i = 1; i <= n; ++i)
        std::cout << "vertex " << i << " order " << order[i] << std::endl; */
    unsigned int* greedy_order = new unsigned int[n + 1];
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
                nodes[p_edge->node_id].node_status = _DOMINATED;
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

void graph::greedy_dynamic() {

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
        } else if (nodes[i].node_status == _DOMINATED && maximal) {
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

void graph::add_vertex(const unsigned int& u) {
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
}

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
    edge* p_edge_u = new edge(v);
    if (nodes[u].edges != NULL)
        p_edge_u->next_edge = nodes[u].edges;
    nodes[u].edges = p_edge_u;

    nodes[v].degree++;
    edge* p_edge_v = new edge(u);
    if (nodes[v].edges != NULL)
        p_edge_v->next_edge = nodes[v].edges;
    nodes[v].edges = p_edge_v;

    if (nodes[u].node_status == _MIS && nodes[v].node_status == _MIS) {
        if (nodes[u].degree < nodes[v].degree) swap(u, v);
        vector<unsigned int> v_in, v_out;
        localserach_one_imp(u, v_in, v_out);
        if (v_in.size() > v_out.size()) {
            // update_neighbors(u);
            update_neighbors(v_in, v_out);
            return;
        }

        v_in.clear();
        v_out.clear();
        localserach_one_imp(u, v_in, v_out);
        if (v_in.size() > v_out.size()) {
            // update_neighbors(v);
            update_neighbors(v_in, v_out);
            return;
        }

        update_neighbors(u);

    } else if (nodes[u].node_status == _MIS && nodes[v].node_status == _DOMINATED)
        nodes[v].counter++;
    else if (nodes[u].node_status == _DOMINATED && nodes[v].node_status == _MIS)
        nodes[u].counter++;
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
    edge* p_edge_u = nodes[u].edges;
    edge* p_pre_edge_u = p_edge_u;
    while (p_edge_u != NULL) {
        if (p_edge_u->node_id == v) {
            if (p_pre_edge_u == p_edge_u)
                nodes[u].edges = p_edge_u->next_edge;
            else
                p_pre_edge_u->next_edge = p_edge_u->next_edge;
            delete p_edge_u;
            nodes[u].degree--;
            // cout << "delete edge: (" << u << ", " << v << ")\n";
            break;
        }
        p_pre_edge_u = p_edge_u;
        p_edge_u = p_edge_u->next_edge;
    }
    if (p_edge_u == NULL) {
        cout << "edge (" << u << ", " << v << ") does not exist.\n";
        return;
    }

    edge* p_edge_v = nodes[v].edges;
    edge* p_pre_edge_v = p_edge_v;
    while (p_edge_v != NULL) {
        if (p_edge_v->node_id == u) {
            if (p_pre_edge_v == p_edge_v)
                nodes[v].edges = p_edge_v->next_edge;
            else
                p_pre_edge_v->next_edge = p_edge_v->next_edge;
            delete p_edge_v;
            nodes[v].degree--;
            // cout << "delete edge: (" << v << ", " << u << ")\n";
            break;
        }
        p_pre_edge_v = p_edge_v;
        p_edge_v = p_edge_v->next_edge;
    }

    if (p_edge_v == NULL) {
        cout << "edge (" << u << ", " << v << ") does not exist.\n";
        return;
    }

    if (nodes[u].node_status == _DOMINATED && nodes[v].node_status == _MIS)
        swap(u, v);
    if (nodes[u].node_status == _MIS && nodes[v].node_status == _DOMINATED) {
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
    }
}

void graph::update_neighbors(const unsigned int& u) {
    nodes[u].node_status = _DOMINATED;
    nodes[u].counter = 1;
    mis--;
    edge* p_edge_u = nodes[u].edges;
    while(p_edge_u != NULL) {
        nodes[p_edge_u->node_id].counter--;
        if (nodes[p_edge_u->node_id].counter == 0) {
            nodes[p_edge_u->node_id].node_status = _MIS;
            mis++;
            edge* p_edge_w = nodes[p_edge_u->node_id].edges;
            while (p_edge_w != NULL) {
                nodes[p_edge_w->node_id].counter++;
                p_edge_w = p_edge_w->next_edge;
            }
        }
        p_edge_u = p_edge_u->next_edge;
    }
}

void graph::update_neighbors(const vector<unsigned int>& v_in, const vector<unsigned int>& v_out) {
    // update information of v_out
    for (int i = 0; i < v_out.size(); ++i) {
        unsigned int id = v_out[i];
        nodes[id].node_status = _UNVISITD;
        nodes[id].counter = 0;
        edge* p_edge = nodes[id].edges;
        while (p_edge != NULL) {
            if (nodes[p_edge->node_id].node_status == _MIS) {
                nodes[id].node_status = _DOMINATED;
                nodes[id].counter++;
            }
        }
    }
}


/*
 * find maximum v_in with no conflict
 */
void graph::localserach_one_imp(const unsigned int& u, vector<unsigned int>& v_in, vector<unsigned int>& v_out) {
    if (u <= 0 || u > n || nodes[u].node_status == _DELETED) {
        cout << "vertex " << u << " does not exist.\n";
        return;
    }
    v_out.push_back(u);
    // find candidates


}

void graph::localsearch_two_imp(const unsigned int& u, vector<unsigned int>& v_in, vector<unsigned int>& v_out) {
     if (u <= 0 || u > n || nodes[u].node_status == _DELETED) {
        cout << "vertex " << u << " does not exist.\n";
        return;
     }


}

/*
 * test for graph.h
 */
int main(int argc, char *argv[])
{
    // std::cout << argv[1] << std::endl;
    graph g(argv[1]);
    g.read_graph();
    // g.show();
    // g.test();
    g.greedy();
    return 0;
}
