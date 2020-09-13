
#include "utility.h"

#define _LONG_ID_
#ifdef _LONG_ID_
    typedef unsigned long ui;
#else
    typedef unsigned int ui;
#endif

#define _NODES_PER_BLOCK (_BLOCK_SIZE/sizeof(ui) - sizeof(off_t)/sizeof(ui))

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

struct page_t {
    ui _neighbors[_NODES_PER_BLOCK];
    off_t _next;

    page_t() {
        memset(_neighbors, 0, sizeof(ui)*(_NODES_PER_BLOCK));
        _next = 0;
    }
};

class ex_graph {
    string _file_path;
    string _mis_path;
    ui _n, _m, _mis;
    status *_status;
    ui *_counter;
    off_t *_offset;

    long long _utime;
    long long _io_cnt;

    FILE *_fp;
    ui _fp_level;
    off_t _slot; // which page to store new block, start from 1
    vector<off_t> _recycle;

    void _open_file(const char *mode="rb+");
    void _close_file();
    off_t _alloc_page();
    void _unalloc_page(off_t block_id);
    int _map(page_t *r_page, off_t block_id);
    int _unmap(page_t *w_page, off_t block_id);

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
    ex_graph(const char *file_path, const char *mis_path);
    ~ex_graph();

    void read_graph();

    void show();
    void show(const ui& node_id);

    void experiment(const char *inst_path);

};

