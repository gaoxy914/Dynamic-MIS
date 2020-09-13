#ifndef _EX_GRAPH_H_
#define _EX_GRAPH_H_

#include "utility.h"

// #define _LONG_ID_

#ifdef _LONG_ID_
    typedef unsigned long ui;
#else
    typedef unsigned int ui;
#endif

#define _NODES_PER_BLOCK (_BLOCK_SIZE/sizeof(ui))

using namespace std;

struct update_t {
    int type;
    ui u;
    ui v;
    update_t(int _type = -1, ui _u = 0, ui _v = 0)
        : type(_type), u(_u), v(_v) {}
};

enum status {
    _UNVISITED, _MIS, _NOMIS, _CONFLICT, _CANDIDATE, _AFFECTED, _SWAPPABLE, _DELETED
};


// graph representation: page list
class ex_graph {
    string _file_path;
    string _mis_path;

    ui _n, _m, _mis;

    status *_status;
    ui *_counter;

    long long _utime;
    long long _io_cnt;

    ui *_offset;
    blockmanager *_bm;

    void _read_neighbors(ulong block_id, ui* neighbors);
    void _write_neighbors(ulong block_id, ui* neighbors);

    void _check_mis();
    void _check_swap();

    void _handle_vertex_deletion(const ui& u);
    void _handle_edge_addition(ui u, ui v);
    void _handle_edge_deletion(ui u, ui v);

    void _add_into_IS(const ui& u);

    void _add_vertex(const ui& u);
    void _delete_vertex(const ui& u);
    void _add_edge(const ui& u, const ui& v);
    void _delete_edge(const ui& u, const ui& v);

    // force: 1: must enlarge, 0: otherwise
    ui _one_swap(const ui& u, bool force);
    ui _two_swap(const ui& u, ui& v);

public:
    ex_graph(const char *_file_path, const char *_mis_path);
    ~ex_graph();

    void read_graph();

    void show();
    void show(ui node_id);

    void experiment(const char *_inst_path);
};

#endif
