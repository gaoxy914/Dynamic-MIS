#include "ex_graph.h"

void ex_graph::_open_file(const char *mode) {
    if (_fp_level == 0)
        _fp = fopen((_GRAPHDISK_PATH + _file_path).c_str(), mode);
    ++ _fp_level;
}

void ex_graph::_close_file() {
    if (_fp_level == 1)
        fclose(_fp);
    -- _fp_level;
}

off_t ex_graph::_alloc_page() {
    off_t slot = _slot;
    if (!_recycle.empty()) {
        slot = _recycle.back();
        _recycle.pop_back();
    } else
        ++ _slot;
    return slot;
}

void ex_graph::_unalloc_page(off_t block_id) {
    _recycle.push_back(block_id);
}

int ex_graph::_map(page_t *r_page, off_t block_id) {
    _io_cnt ++;
    _open_file();
    fseek(_fp, block_id*_BLOCK_SIZE, SEEK_SET);
    size_t rd = fread(r_page, _BLOCK_SIZE, 1, _fp);
    _close_file();
    return rd - 1;
}

int ex_graph::_unmap(page_t *w_page, off_t block_id) {
    _io_cnt ++;
    _open_file();
    fseek(_fp, block_id*_BLOCK_SIZE, SEEK_SET);
    size_t wd = fwrite(w_page, _BLOCK_SIZE, 1, _fp);
    _close_file();
    return wd - 1;
}

void ex_graph::_check_mis() {
    ui cnt = 0;
    bool maximal = true;
    page_t page;
    for (ui i = 1; i <= _n; ++ i) {
        if (_status[i] == _MIS) {
            cnt ++;
            off_t block_id = _offset[i];
            while (block_id != 0) {
                _map(&page, block_id);
                for (int j = 0; j < _NODES_PER_BLOCK; ++ j) {
                    if (page._neighbors[j] == 0) break;
                    if (_status[page._neighbors[j]] == _MIS) {
                        cout << "CHECK_IS: WA two adjacent vertices " 
                            << i << ", " << page._neighbors[j] << " in MIS.\n";
                        exit(1);
                    }
                }
                block_id = page._next;
            }
        } else if (_status[i] == _NOMIS) {
            ui mis_cnt = 0;
            bool find  = false;
            off_t block_id = _offset[i];
            while (block_id != 0) {
                _map(&page, block_id);
                for (int j = 0; j < _NODES_PER_BLOCK; ++ j) {
                    if (page._neighbors[j] == 0) break;
                    if (_status[page._neighbors[j]] == _MIS) {
                        find = true;
                        mis_cnt ++;
                    }
                }
                block_id = page._next;
            }
            if (!find) {
                cout << "CHECK_IS: WA " << i << " not maximal.\n";
                exit(1);
            }
            if (mis_cnt != _counter[i]) {
                cout << "CHECK_IS WA " << i << " wrong counter " << mis_cnt << ", " << _counter[i] << endl;
                exit(1);
            }
        } else if (_status[i] == _DELETED) {
            if (_counter[i] != 0) {
                cout << "CHECK_IS WA " << i << " wrong deleted counter.\n";
                exit(1);
            }
        } else {
            cout << "CHECK_IS WRONG STATUS vertex " << i << " status " << _status[i] << endl;
            exit(1);
        }
    }
    cout << "|MIS| " << cnt << ", " << _mis << endl;
}

void ex_graph::_check_swap() {
        ui swap_cnt = 0;
    ui swap_cnt_2 = 0;
    for (ui i = 1; i <= _n; ++ i) {
        if (_status[i] == _MIS) {
            if (_one_swap(i, true) > 1) {
                swap_cnt ++;
            }
#ifdef _TWO_SWAP_
            ui v = 0;
            if (_two_swap(i, v) > 2) {
                swap_cnt_2 ++;
            }
#endif
        }
    }
    cout << "CHECK_SWAP: |swappable|: " << swap_cnt
        << "|2-swappable|: " << swap_cnt_2 << ", after swap |MIS|: " << _mis << endl;

}

void ex_graph::_handle_vertex_deletion(const ui& u) {
#ifdef _LINUX_
    struct timeval start, end;
    gettimeofday(&start, NULL);
#endif
    if (_status[u] == _MIS) {
        ui mis_cnt = 0, v = 0;
        if (_one_swap(u, false) > 0) {
#ifndef NDEBUG
            cout << "one swap success.\n";
            cout << "node " << u << ", status, " << _status[u] << endl;
#endif
        }
#ifdef _TWO_SWAP_
        else if (_two_swap(u, v) >= 2) {
#ifndef NDEBUG
            cout << "two swap success.\n";
            cout << "node " << u << ", status, " << _status[u]
                << ", node " << v << ", status " << _status[v] << endl;
#endif
        }
#endif
        else {
#ifndef NDEBUG
            cout << "remove " << u << " from the solution.\n";
#endif
            _status[u] = _DELETED;
            _mis --;
            off_t block_id = _offset[u];
            page_t page;
            while (block_id != 0) {
                _map(&page, block_id);
                for (int j = 0; j < _NODES_PER_BLOCK; ++ j) {
                    ui v = page._neighbors[j];
                    if (v == 0) break;
                    if (_status[v] != _DELETED) {
                        _counter[v] --;
                        if (_counter[v] == 0) {
                            _add_into_IS(v);
                            _mis ++;
                        }
                        if (_counter[v] == 1) {
                            _status[v] = _SWAPPABLE;
                        }
#ifdef _TWO_SWAP_
                        else if (_counter[v] == 2) {
                            _status[v] = _SWAPPABLE;
                        }
#endif
                    }
                }
                block_id = page._next;
            }

            for (ui i = 1; i <= _n; ++ i) {
                if (_status[i] == _SWAPPABLE) {
                    _status[i] = _NOMIS;
                    if (_counter[i] == 1) {
                        ui w = 0;
                        for (block_id = _offset[i]; block_id != 0; block_id = page._next) {
                            _map(&page, block_id);
                            for (int j = 0; j < _NODES_PER_BLOCK; ++ j) {
                                if (page._neighbors[j] == 0) break;
                                if (_status[page._neighbors[j]] == _MIS) {
                                    w = page._neighbors[j];
                                    page._next = 0;
                                    break;
                                }
                            }
                        }
                        _one_swap(w, true);
                    }
#ifdef _TWO_SWAP_
                    else if (_counter[i] == 2) {
                        ui x = 0, y = 0;
                        for (block_id = _offset[i]; block_id != 0; block_id = page._next) {
                            _map(&page, block_id);
                            for (int j = 0; j < _NODES_PER_BLOCK; ++ j) {
                                if (page._neighbors[j] == 0) break;
                                if (_status[page._neighbors[j]] == _MIS) {
                                    if (!x) x = page._neighbors[j];
                                    else {
                                        y = page._neighbors[j];
                                        page._next = 0;
                                        break;
                                    }
                                }
                            }
                        }
                        _two_swap(x, y);
                    }
#endif
                }
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

void ex_graph::_handle_edge_addition(ui u, ui v) {
#ifdef _LINUX_
    struct timeval start, end;
    gettimeofday(&start, NULL);
#endif
    if (_status[u] == _NOMIS && _status[v] == _MIS)
        std::swap(u, v);
#ifndef NDEBUG
    cout << "node " << u << ", status, " << _status[u]
        << ", node " << v << ", status " << _status[v] << endl;
#endif
    _add_edge(u, v);
    _add_edge(v, u);
    if (_status[u] == _MIS && _status[v] == _MIS) {
        _status[u] = _CONFLICT;
        _status[v] = _CONFLICT;
        _counter[u] = 1;
        _counter[v] = 1;
        ui x = 0, y = 0;

        if (_one_swap(u, false) || _one_swap(v, false)) {
#ifndef NDEBUG
            cout << "one swap success.\n";
            cout << "node " << u << ", status, " << _status[u]
                << ", node " << v << ", status " << _status[v] << endl;
#endif
            if (_status[u] == _MIS) _one_swap(u, true);
            if (_status[v] == _MIS) _one_swap(v, true);
        }
#ifdef _TWO_SWAP_
        else if (_two_swap(u, x) >= 2 || _two_swap(v, y) >= 2) {
#ifndef NDEBUG
            cout << "two swap success.\n";
            cout << "node " << u << ", status, " << _status[u]
                << ", node " << v << ", status " << _status[v] << endl;
#endif
        }
#endif
        else {
#ifndef NDEBUG
            cout << "node " << u << ", status, " << _status[u]
                << ", node " << v << ", status " << _status[v] << endl;
#endif
            if (_offset[u + 1] - _offset[u] < _offset[v + 1] - _offset[v])
                std::swap(u, v);
#ifndef NDEBUG
            cout << "remove " << u << " from the solution.\n";
#endif
            _status[u] = _NOMIS;
            _status[v] = _MIS;
            _counter[u] = 1;
            _counter[v] = 0;
            _mis --;
            page_t page;
            for (off_t block_id = _offset[u]; block_id != 0; block_id = page._next) {
                _map(&page, block_id);
                for (int j = 0; j < _NODES_PER_BLOCK; ++ j) {
                    ui v = page._neighbors[j];
                    if (v == 0) break;
                    if (_status[v] != _DELETED) {
                        if (_status[v] == _NOMIS)
                            _counter[v] --;
                        if (_counter[v] == 1)
                            _status[v] = _SWAPPABLE;
#ifdef _TWO_SWAP_
                        else if (_counter[v] == 2)
                            _status[v] = _SWAPPABLE;
#endif
                    }
                }
            }

            for (ui i = 1; i <= _n; ++ i) {
                if (_status[i] == _SWAPPABLE) {
                    _status[i] = _NOMIS;
                    if (_counter[i] == 1) {
                        ui w = 0;
                        for (off_t block_id = _offset[i]; block_id != 0; block_id = page._next) {
                            _map(&page, block_id);
                            for (int j = 0; j < _NODES_PER_BLOCK; ++ j) {
                                if (page._neighbors[j] == 0) break;
                                if (_status[page._neighbors[j]] == _MIS) {
                                    w = page._neighbors[j];
                                    page._next = 0;
                                    break;
                                }
                            }
                        }
                        _one_swap(w, true);
                    }
#ifdef _TWO_SWAP_
                    else if (_counter[i] == 2) {
                        ui x = 0, y = 0;
                        for (off_t block_id = _offset[i]; block_id != 0; block_id = page._next) {
                            _map(&page, block_id);
                            for (int j = 0; j < _NODES_PER_BLOCK; ++ j) {
                                if (page._neighbors[j] == 0) break;
                                if (_status[page._neighbors[j]] == _MIS) {
                                    if (!x) x = page._neighbors[j];
                                    else {
                                        y = page._neighbors[j];
                                        page._next = 0;
                                        break;
                                    }
                                }
                            }
                        }

                        _two_swap(x, y);
                    }
#endif
                }
            }
        }
    } else if (_status[u] == _MIS && _status[v] == _NOMIS)
        _counter[v] ++;

#ifdef _LINUX_
    gettimeofday(&end, NULL);
    long long mtime, seconds, useconds;
    seconds = end.tv_sec - start.tv_sec;
    useconds = end.tv_usec - start.tv_usec;
    mtime = seconds*1000000 + useconds;
    _utime += mtime;
#endif
}

void ex_graph::_handle_edge_deletion(ui u, ui v) {
#ifdef _LINUX_
    struct timeval start, end;
    gettimeofday(&start, NULL);
#endif
    if (_status[u] == _NOMIS && _status[v] == _MIS)
        std::swap(u, v);
    _delete_edge(u, v);
    _delete_edge(v, u);
    if (_status[u] == _MIS && _status[v] == _NOMIS) {
        _counter[v] --;
        if (_counter[v] == 0) {
            _add_into_IS(v);
            _mis ++;
        } else if (_counter[v] == 1) {
            ui w = 0;
            page_t page;
            for (off_t block_id = _offset[v]; block_id != 0; block_id = page._next) {
                _map(&page, block_id);
                for (int j = 0; j < _NODES_PER_BLOCK; ++ j) {
                    if (page._neighbors[j] == 0) break;
                    if (_status[page._neighbors[j]] == _MIS) {
                        w = page._neighbors[j];
                        page._next = 0;
                        break;
                    }
                }
            }
            // cout << "w " << w << endl;
            _one_swap(w, true);
        }
#ifdef _TWO_SWAP_
        else if (_counter[v] == 2) {
            ui x = 0, y = 0;
            page_t page;
            for (off_t block_id = _offset[v]; block_id != 0; block_id = page._next) {
                _map(&page, block_id);
                for (int j = 0; j < _NODES_PER_BLOCK; ++ j) {
                    if (page._neighbors[j] == 0) break;
                    if (_status[page._neighbors[j]] == _MIS) {
                        if (!x) x = page._neighbors[j];
                        else {
                            y = page._neighbors[j];
                            page._next = 0;
                            break;
                        }
                    }
                }
            }

            _two_swap(x, y);
        }
#endif
    } else {
        if (_counter[u] == 1 && _counter[v] == 1) {
            ui w = 0;
            page_t page;
            for (off_t block_id = _offset[v]; block_id != 0; block_id = page._next) {
                _map(&page, block_id);
                for (int j = 0; j < _NODES_PER_BLOCK; ++ j) {
                    if (page._neighbors[j] == 0) break;
                    if (_status[page._neighbors[j]] == _MIS) {
                        w = page._neighbors[j];
                        page._next = 0;
                        break;
                    }
                }
            }
            _one_swap(w, true);
        }
#ifdef _TWO_SWAP_
        else if (max(_counter[u], _counter[v]) == 2) {
            if (_counter[v] < 2) swap(u, v);
            ui x = 0, y = 0;
            page_t page;
            for (off_t block_id = _offset[v]; block_id != 0; block_id = page._next) {
                _map(&page, block_id);
                for (int j = 0; j < _NODES_PER_BLOCK; ++ j) {
                    if (page._neighbors[j] == 0) break;
                    if (_status[page._neighbors[j]] == _MIS) {
                        if (!x) x = page._neighbors[j];
                        else {
                            y = page._neighbors[j];
                            page._next = 0;
                            break;
                        }
                    }
                }
            }

            _two_swap(x, y);
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

void ex_graph::_add_into_IS(const ui& u) {
#ifndef NDEBUG
    cout << "adding " << u << " into IS.\n";
#endif
    _status[u] = _MIS;
    _counter[u] = 0;
    page_t page;
    for (off_t block_id = _offset[u]; block_id != 0; block_id = page._next) {
        _map(&page, block_id);
        for (ui j = 0; j < _NODES_PER_BLOCK; ++ j) {
            ui v = page._neighbors[j];
            if (v == 0) break;
            if (_status[v] == _CANDIDATE)
                _counter[v] = 1;
            else if (_status[v] != _DELETED)
                _counter[v] ++;
            if (_status[v] != _AFFECTED && _status[v] != _DELETED)
                _status[v] = _NOMIS;
        }
    }
}

void ex_graph::_add_vertex(const ui& u) {
    if (u <= 0 || u > _n + _INST_NUM) {
        cout << "vertex number " << u << " out of range.\n";
        return;
    }
}

void ex_graph::_delete_vertex(const ui& u) {
    _status[u] = _DELETED;
    _counter[u] = 0;
}

void ex_graph::_add_edge(const ui& u, const ui& v) {
    page_t page_u;
    if (_offset[u] == 0) { // no page_t before, alloc a new page_t
        off_t block_id = _alloc_page();
        page_u._neighbors[0] = v;
        _unmap(&page_u, block_id);
        _offset[u] = block_id;
    } else {
        off_t block_id = _offset[u];
        _map(&page_u, block_id);
        if (page_u._neighbors[_NODES_PER_BLOCK - 1] == 0) {
            for (int i = 0; i < _NODES_PER_BLOCK; ++ i) {
                if (page_u._neighbors[i] == 0) {
                    page_u._neighbors[i] = v;
                    _unmap(&page_u, block_id);
                    break;
                }
            }
        } else { // full page_t, alloc a new page_t
            off_t new_bld = _alloc_page();
            cout << new_bld << endl;
            page_t new_page;
            new_page._neighbors[0] = v;
            new_page._next = block_id;
            _unmap(&new_page, new_bld);
            _offset[u] = new_bld;
        }
    }
}

void ex_graph::_delete_edge(const ui& u, const ui& v) {
    page_t page_u;
    off_t block_id = _offset[u];
    off_t v_bid = 0; int v_pos = 0;
    ui tmp = 0;
    _map(&page_u, block_id);
    int i = 1;
    for ( ; i < _NODES_PER_BLOCK; ++ i)
        if (page_u._neighbors[i] == 0) {
            swap(tmp, page_u._neighbors[i - 1]);
            break;
        }
    if (i == 1) { // recycle empty page_t
        _offset[u] = page_u._next;
        // cout << "recycle " << block_id << endl;
        _unalloc_page(block_id);
    } else
        _unmap(&page_u, block_id);
    if (tmp != v) {
        block_id = _offset[u];
        while (block_id != 0) {
            _map(&page_u, block_id);
            for (int i = 0; i < _NODES_PER_BLOCK; ++ i) {
                if (page_u._neighbors[i] == v) {
                    swap(tmp, page_u._neighbors[i]);
                    _unmap(&page_u, block_id);
                    page_u._next = 0;
                    break;
                }
            }
            block_id = page_u._next;
        }
    }
}

ui ex_graph::_one_swap(const ui& u, bool force) {
#ifndef NDEBUG
    cout << "one swap " << u << endl;
#endif
    ui cnt = 0, mis_cnt = 0;
    page_t page;
    for (off_t block_id = _offset[u]; block_id != 0; block_id = page._next) {
        _map(&page, block_id);
        for (int j = 0; j < _NODES_PER_BLOCK; ++ j) {
            ui v = page._neighbors[j];
            if (v == 0) break;
            if (_status[v] == _NOMIS || _status[v] == _SWAPPABLE) {
                _status[v] = _AFFECTED;
                if (_counter[v] == 1) {
                    _status[v] = _CANDIDATE;
                    cnt ++;
                }
            }
        }
    }

    if (cnt == 0 || (cnt == 1 && force)) {
        for (ui i = 1; i <= _n; ++ i)
            if (_status[i] == _AFFECTED || _status[i] == _CANDIDATE)
                _status[i] = _NOMIS;
        return mis_cnt;
    }
    if (cnt == 1 && !force) {
        _status[u] = _NOMIS;
        for (ui i = 1; i <= _n; ++ i) {
            if (_status[i] == _CANDIDATE)
                _add_into_IS(i);
            else if (_status[i] == _AFFECTED) {
                _status[i] = _NOMIS;
                _counter[i] --;
            } else if (_status[i] == _CONFLICT) {
                _status[i] = _MIS;
                _counter[i] --;
            }
        }
        mis_cnt = 1;
        return mis_cnt;
    }
    bool enlarge = false;
    ui *degree_buckets = new ui[cnt + 1];
    memset(degree_buckets, 0, sizeof(ui)*(cnt + 1));
    for (ui i = 1; i <= _n; ++ i) {
        if (_status[i] == _CANDIDATE) {
            _counter[i] = 0;
            for (off_t block_id = _offset[i]; block_id != 0; block_id = page._next) {
                _map(&page, block_id);
                for (ui j = 0; j < _NODES_PER_BLOCK; ++ j) {
                    if (page._neighbors[j] == 0) break;
                    if (_status[page._neighbors[j]] == _CANDIDATE)
                        _counter[i] ++;
                }
            }

            degree_buckets[_counter[i]] ++;
            if (_counter[i] < cnt - 1) {
                enlarge = true;
            }
        }
    }
    if (!force || enlarge) {
        _status[u] = _NOMIS;
        for (ui i = 1; i < cnt; ++ i)
            degree_buckets[i] += degree_buckets[i - 1];

        for (ui i = 1; i <= _n; ++ i)
            if (_status[i] == _CANDIDATE)
                _counter[i] = degree_buckets[_counter[i]] --;

        memset(degree_buckets, 0, sizeof(ui)*(cnt + 1));
        for (ui i = 1; i <= _n; ++ i)
            if (_status[i] == _CANDIDATE)
                degree_buckets[_counter[i]] = i;

        for (ui i = 1; i <= cnt; ++ i) {
            if (_status[degree_buckets[i]] == _CANDIDATE) {
                _add_into_IS(degree_buckets[i]);
                mis_cnt ++;
            }
        }

        for (ui i = 1; i <= _n; ++ i) {
            if (_status[i] == _AFFECTED) {
                _status[i] = _NOMIS;
                _counter[i] --;
            } else if (_status[i] == _CONFLICT) {
                _status[i] = _MIS;
                _counter[i] = 0;
            }
        }

        _mis += (mis_cnt - 1);
    } else {
        for (ui i = 1; i <= _n; ++ i) {
            if (_status[i] == _CANDIDATE) {
                _status[i] = _NOMIS;
                _counter[i] = 1;
            } else if (_status[i] == _AFFECTED)
                _status[i] = _NOMIS;
        }
    }
    delete[] degree_buckets;
#ifndef NDEBUG
    cout << "remove " << u << " from the solution.\n";
    cout << "mis_cnt " << mis_cnt << endl;
#endif
    return mis_cnt;
}

ui ex_graph::_two_swap(const ui& u, ui& v) {
#ifndef NDEBUG
    cout << "two swap " << u << ", " << v << endl;
#endif
    ui cnt = 0, mis_cnt = 0;
    page_t page;
    for (off_t block_id = _offset[u]; block_id != 0; block_id = page._next) {
        _map(&page, block_id);
        for (int j = 0; j < _NODES_PER_BLOCK; ++ j) {
            ui x = page._neighbors[j];
            if (x == 0) break;
            if (_status[x] == _NOMIS || _status[x] == _SWAPPABLE) {
                _status[x] = _NOMIS;
                if (_counter[x] == 2) { // find another IS-neighbor of x except u
                    ui y = 0;
                    page_t page_x;
                    for (off_t block_id_x = _offset[x]; block_id_x != 0; block_id_x = page_x._next) {
                        _map(&page_x, block_id_x);
                        for (int k = 0; k < _NODES_PER_BLOCK; ++ k) {
                            if (page_x._neighbors[k] == 0) break;
                            if (page_x._neighbors[k] != u) {
                                if (_status[page_x._neighbors[k]] == _MIS) {
                                    y = page_x._neighbors[k];
                                    _counter[y] ++;
                                    page_x._next = 0;
                                    break;
                                }
                            }
                        }
                    }
                    _status[x] = _CANDIDATE;
                    _counter[x] = y;
                }
            }
        }
    }
    // show();
    if (v == 0) {
        ui max_cnt = 0;
        for (ui i = 0; i <= _n; ++ i) {
            if (_status[i] == _MIS) {
                if (_counter[i] > max_cnt) {
                    max_cnt = _counter[i];
                    v = i;
                }
                _counter[i] = 0;
            }
        }
    } else {
        for (ui i = 0; i <= _n; ++ i)
            if (_status[i] == _MIS)
                _counter[i] = 0;
    }

    if (v == 0) return 0;

    unordered_map<ui, ui> backup;
    for (ui i = 0; i <= _n; ++ i) {
        if (_status[i] == _CANDIDATE) {
            if (_counter[i] != v) _status[i] = _NOMIS;
            else {
                backup[i] = 2;
                cnt ++;
            }
            _counter[i] = 2;
        }
    }

    for (off_t block_id = _offset[u]; block_id != 0; block_id = page._next) {
        _map(&page, block_id);
        for (int j = 0; j < _NODES_PER_BLOCK; ++ j) {
            ui x = page._neighbors[j];
            if (x == 0) break;
            if (_status[x] == _NOMIS || _status[x] == _SWAPPABLE) {
                _status[x] = _AFFECTED;
                backup[x] = 1;
                if (_counter[x] == 1) {
                    _status[x] = _CANDIDATE;
                    cnt ++;
                }
            }
        }
    }

    for (off_t block_id = _offset[v]; block_id != 0; block_id = page._next) {
        _map(&page, block_id);
        for (int j = 0; j < _NODES_PER_BLOCK; ++ j) {
            ui x = page._neighbors[j];
            if (x == 0) break;
            if (_status[x] == _NOMIS || _status[x] == _SWAPPABLE) {
                _status[x] = _AFFECTED;
                backup[x] = 1;
                if (_counter[x] == 1) {
                    _status[x] = _CANDIDATE;
                    // backup[neighbors[j]] = 1;
                    cnt ++;
                }
            } else if (_status[x] == _AFFECTED) {
                backup[x] += 1;
            }
        }
    }

/* #ifndef NDEBUG
    cout << "cnt " << cnt << endl;

    for (auto iter : backup) {
        printf("node %d, status %d, backup %d.\n", iter.first, _status[iter.first], iter.second);
    }
#endif */

    if (cnt > 1) {
        bool swappable = false;
        ui *degree_buckets = new ui[cnt + 1];
        memset(degree_buckets, 0, sizeof(ui)*(cnt + 1));
        for (ui i = 1; i <= _n; ++ i) {
            if (_status[i] == _CANDIDATE) {
                _counter[i] = 0;
                for (off_t block_id = _offset[i]; block_id != 0; block_id = page._next) {
                    _map(&page, block_id);
                    for (int j = 0; j < _NODES_PER_BLOCK; ++ j) {
                        if (page._neighbors[j] == 0) break;
                        if (_status[page._neighbors[j]] == _CANDIDATE)
                            _counter[i] ++;
                    }
                }

                degree_buckets[_counter[i]] ++;
                if (_counter[i] < cnt - 1) {
                    swappable = true;
                }
            }
        }

        if (swappable) {
            _status[u] = _NOMIS;
            _status[v] = _NOMIS;
            for (ui i = 1; i < cnt; ++ i)
                degree_buckets[i] += degree_buckets[i - 1];

            for (ui i = 1; i <= _n; ++ i)
                if (_status[i] == _CANDIDATE)
                    _counter[i] = degree_buckets[_counter[i]] --;

            memset(degree_buckets, 0, sizeof(ui)*(cnt + 1));
            for (ui i = 1; i <= _n; ++ i)
                if (_status[i] == _CANDIDATE)
                    degree_buckets[_counter[i]] = i;


            for (ui i = 1; i <= cnt; ++ i) {
                if (_status[degree_buckets[i]] == _CANDIDATE) {
                    _add_into_IS(degree_buckets[i]);
                    mis_cnt ++;
                }
            }
            for (ui i = 1; i <= _n; ++ i) {
                if (_status[i] == _AFFECTED) {
                    _status[i] = _NOMIS;
                    _counter[i] -= backup[i];
                } else if (_status[i] == _CONFLICT) {
                    _status[i] = _MIS;
                    _counter[i] = 0;
                }
            }

            _mis += (mis_cnt - 2);
        } else {
            for (ui i = 1; i <= _n; ++ i) {
                if (_status[i] == _CANDIDATE) {
                    _status[i] = _NOMIS;
                    _counter[i] = backup[i];
                } else if (_status[i] == _AFFECTED)
                    _status[i] = _NOMIS;
            }
        }
        delete[] degree_buckets;
    } else {
        for (ui i = 1; i <= _n; ++ i) {
            if (_status[i] == _CANDIDATE || _status[i] == _AFFECTED)
                _status[i] = _NOMIS;
        }
    }
    return mis_cnt;
}

ex_graph::ex_graph(const char *file_path, const char *mis_path) {
    _file_path = (string)file_path;
    _mis_path = (string)mis_path;
    _n = _m = _mis = 0;
    _offset = NULL;
    _status = NULL;
    _counter = NULL;
    _utime = 0;
    _io_cnt = 0;
    _fp = NULL;
    _fp_level = 0;
    _slot = 1;
    _open_file("wb+");
}

ex_graph::~ex_graph() {
    if (_status != NULL) {
        delete[] _status;
        _status = NULL;
    }
    if (_counter != NULL) {
        delete[] _counter;
        _counter = NULL;
    }
    if (_offset != NULL) {
        delete[] _offset;
        _offset = NULL;
    }
    _close_file();
}

/*
 * GRAPH_DISK FORMAT: PAGE_t LIST (NEAR END NOT FULL)
 */
void ex_graph::read_graph() {
    cout << "reading graph.\n";
    ifstream infile_mis((_MIS_PATH + _mis_path).c_str());
    ifstream infile_g((_GRAPH_PATH + _file_path).c_str());
    infile_g >> _n >> _m;
    cout << "n = " << _n << ", m = " << _m
        << ", nodes per block = " << _NODES_PER_BLOCK << endl;
    if (_status == NULL) {
        _status = new status[_n + 1];
        memset(_status, _UNVISITED, sizeof(status)*(_n + 1));
    }
    if (_counter == NULL) {
        _counter = new ui[_n + 1];
        memset(_counter, 0, sizeof(ui)*(_n + 1));
    }
    if (_offset == NULL) {
        _offset = new off_t[_n + 1];
        memset(_offset, 0, sizeof(off_t)*(_n + 1));
    }
    infile_mis >> _mis;
    cout << "initial mis = " << _mis << endl;
    ui u = 0;
    for (ui i = 0; i < _mis; ++ i) {
        infile_mis >> u;
        _status[u + 1] = _MIS;
    }
    infile_mis.close();
    for (ui i = 1; i <= _n; ++ i) {
        ui degree = 0;
        infile_g >> degree;
        // cout << i << ", " << degree << endl;
        if (degree > 0) {
            ui block_num = ceil((double)degree/(_NODES_PER_BLOCK));
            // cout << block_num << endl;
            ui n = block_num*(_NODES_PER_BLOCK);
            ui *neis = new ui[n];
            memset(neis, 0, sizeof(ui)*n);
            for (ui j = 0; j < degree; ++ j) {
                infile_g >> neis[j];
                ++ neis[j];
                // cout << neis[j] << ", ";
            }
            // cout << endl;
            if (_status[i] == _MIS)
                for (ui j = 0; j < degree; ++ j) {
                    _counter[neis[j]] += 1;
                    _status[neis[j]] = _NOMIS;
                }

            off_t block_id = 0, next_bld = 0;
            page_t page_i;
            for (ui j = 0; j < block_num; ++ j) {
                block_id = _alloc_page();
                for (int k = 0; k < _NODES_PER_BLOCK; ++ k)
                    page_i._neighbors[k] = neis[j*_NODES_PER_BLOCK + k];

                page_i._next = next_bld;
                _unmap(&page_i, block_id);
                next_bld = block_id;
            }
            _offset[i] = block_id;
            delete[] neis;
        }
    }
    infile_g.close();
    _io_cnt = 0;
}

void ex_graph::show() {
    for (ui i = 1; i <= _n; ++ i) {
        cout << "node id: " << i << ", node status: " << _status[i]
            << ", node counter: " << _counter[i] << ", neighborlist: ";
        off_t block_id = _offset[i];
        while (block_id != 0) {
            page_t page_i;
            _map(&page_i, block_id);
            for (int j = 0; j < _NODES_PER_BLOCK; ++ j) {
                if (page_i._neighbors[j] != 0 && _status[page_i._neighbors[j]] != _DELETED)
                    cout << page_i._neighbors[j] << ", ";
            }
            block_id = page_i._next;
        }
        cout << endl;
    }
}

void ex_graph::show(const ui& node_id) {
    cout << "node id: " << node_id << ", node status: " << _status[node_id]
        << ", node counter: " << _counter[node_id] << ", neighborlist: ";
    off_t block_id = _offset[node_id];
    while (block_id != 0) {
        page_t page;
        _map(&page, block_id);
        for (int j = 0; j < _NODES_PER_BLOCK; ++ j) {
            if (page._neighbors[j] != 0 && _status[page._neighbors[j]] != _DELETED)
                cout << page._neighbors[j] << ", ";
        }
        block_id = page._next;
    }
    cout << endl;
}

void ex_graph::experiment(const char *inst_path) {
    vector<update_t> updates;
    ifstream infile((_INST_PATH + (string)inst_path).c_str());
    if (!infile.is_open()) {
        cout << "instruction file: " << inst_path << " open failed.\n";
        exit(1);
    }
    update_t update;
    for (ui i = 0; i < _INST_NUM; ++ i) {
        infile >> update.type;
        switch (update.type) {
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

    for (ui i = 0; i < _INST_NUM; ++ i) {
#ifndef NDEBUG
        cout << i << ", " << updates[i].type << ", " << updates[i].u + 1 << "," << updates[i].v + 1 << endl;
#endif
        switch (updates[i].type) {
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

    cout << "Update time: " << _utime << "us, |MIS|: " << _mis << ", |I/O|: " << _io_cnt << endl;
#ifndef NDEBUG
    _check_mis();
    // check_swap();
#endif
}

int main(int argc, char const *argv[])
{
    /* code */
    if (argc == 2) {
        string file_path = "ex-" + (string)argv[1] + ".bin";
        string mis_path = "ex-" + (string)argv[1] + ".mis";
        string inst_path = "ex-" + (string)argv[1] + ".inst";
        ex_graph g(file_path.c_str(), mis_path.c_str());
        g.read_graph();
        g.experiment(inst_path.c_str());
    } else if (argc == 4) {
        ex_graph g(argv[1], argv[2]);
        g.read_graph();
        g.experiment(argv[3]);
    } else {
        cout << "error command.\n";
    }
    return 0;
}
