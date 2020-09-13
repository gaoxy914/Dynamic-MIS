#include "ex_graph_os.h"

ex_graph::ex_graph(const char *file_path, const char *mis_path) {
    _file_path = (string)file_path;
    _mis_path = (string)mis_path;
    _n = _m = _mis = 0;
    _utime = 0;
    _io_cnt = 0;
    _status = NULL;
    _counter = NULL;
    _offset = NULL;
    _bm = new blockmanager((_GRAPHDISK_PATH + _file_path).c_str(), O_CREAT, _BLOCK_SIZE);
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
    delete _bm;
}

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
        _offset = new ui[_n + 2];
        memset(_offset, 0, sizeof(ui)*(_n + 2));
    }
    
    infile_mis >> _mis;
    cout << "initial mis = " << _mis << endl;
    ui u = 0;
    for (ui i = 0; i < _mis; ++ i) {
        infile_mis >> u;
        _status[u + 1] = _MIS;
    }
    infile_mis.close();

    ui block_id = 0;
    for (ui i = 1; i <= _n; ++ i) {
        block_id = _offset[i];
        ui degree = 0;
        infile_g >> degree;
        ui u = 0, block_num = ceil((degree + _INST_NUM)/(double)_NODES_PER_BLOCK);
        ui n = block_num*_NODES_PER_BLOCK;
        ui *neighbors = new ui[n];
        memset(neighbors, 0, sizeof(ui)*n);
        for (ui j = 0; j < degree; ++ j) {
            infile_g >> u;
            neighbors[j] = u + 1;
        }
        
        if (_status[i] == _MIS)
            for (ui j = 0; j < degree; ++ j) {
                _counter[neighbors[j]] += 1;
                _status[neighbors[j]] = _NOMIS;
            }
        
        for (ui j = 0; j < block_num; ++ j)
            _write_neighbors(block_id ++, neighbors + j*_NODES_PER_BLOCK);
        
        delete[] neighbors;
        _offset[i + 1] = _offset[i] + block_num;
    }
    infile_g.close();
    _io_cnt = 0;
}

void ex_graph::show() {
    ui *neighbors = new ui[_NODES_PER_BLOCK];
    for (ui i = 1; i <= _n; ++ i) {
        cout << "node id: " << i << ", node status: " << _status[i]
            << ", node counter: " << _counter[i] << ", neighborlist: ";
        for (ui block_id = _offset[i]; block_id < _offset[i + 1]; ++ block_id) {
            _read_neighbors(block_id, neighbors);
            for (ui j = 0; j < _NODES_PER_BLOCK; ++ j) {
                if (neighbors[j] == 0) { block_id = _offset[i + 1]; break; }
                if (_status[neighbors[j]] != _DELETED)
                    cout << neighbors[j] << ", ";
            }
        }
        cout << endl;
    }
    delete[] neighbors;
}

void ex_graph::show(ui node_id) {
    ui *neighbors = new ui[_NODES_PER_BLOCK];
    cout << "node id: " << node_id << ", node status: " << _status[node_id]
        << ", node counter: " << _counter[node_id] << ", neighborlist: ";
    for (ui block_id = _offset[node_id]; block_id < _offset[node_id + 1]; ++ block_id) {
        _read_neighbors(block_id, neighbors);
        for (ui j = 0; j < _NODES_PER_BLOCK; ++ j) {
            if (neighbors[j] == 0) { block_id = _offset[node_id + 1]; break; }
            if (_status[neighbors[j]] != _DELETED)
                cout << neighbors[j] << ", ";
        }
    }
    cout << endl;
    delete[] neighbors;
}

void ex_graph::_read_neighbors(ulong block_id, ui* neighbors) {
    _io_cnt ++;
    memset(neighbors, 0, sizeof(ui)*_NODES_PER_BLOCK);
    u_char *buf = new u_char[_BLOCK_SIZE];
    _bm->read_block(buf, block_id, _BLOCK_SIZE);
    // char2int(_NODES_PER_BLOCK, buf, neighbors);
    for (ui i = 0; i < _NODES_PER_BLOCK; ++ i) {
        ui _offset = i*sizeof(ui);
#ifdef _LONG_ID_
        neighbors[i] = (ui)buf[_offset] | (ui)buf[_offset + 1]<<8
            | (ui)buf[_offset + 2]<<16 | (ui)buf[_offset + 4]<<24
            | (ui)buf[_offset + 4]<<32 | (ui)buf[_offset + 5]<<40
            | (ui)buf[_offset + 6]<<48 | (ui)buf[_offset + 7]<<56;
#else
        neighbors[i] = (ui)buf[_offset] | (ui)buf[_offset + 1]<<8 
            | (ui)buf[_offset + 2]<<16 | (ui)buf[_offset + 3]<<24;
#endif            
    }
    delete[] buf;
}

void ex_graph::_write_neighbors(ulong block_id, ui* neighbors) {
    _io_cnt ++;
    u_char *buf = new u_char[_BLOCK_SIZE];
    memset(buf, 0, sizeof(u_char)*_BLOCK_SIZE);
    // int2char(_NODES_PER_BLOCK, neighbors, buf);
    for (ui i = 0; i < _NODES_PER_BLOCK; ++ i) {
        ui _offset = i*sizeof(ui);
#ifdef _LONG_ID_ 
            buf[_offset] = (u_char)neighbors[i];
            buf[_offset + 1] = (u_char)(neighbors[i]>>8);
            buf[_offset + 2] = (u_char)(neighbors[i]>>16);
            buf[_offset + 3] = (u_char)(neighbors[i]>>24);
            buf[_offset + 4] = (u_char)(neighbors[i]>>32);
            buf[_offset + 5] = (u_char)(neighbors[i]>>40);
            buf[_offset + 6] = (u_char)(neighbors[i]>>48);
            buf[_offset + 7] = (u_char)(neighbors[i]>>56);   
#else
            buf[_offset] = (u_char)neighbors[i];
            buf[_offset + 1] = (u_char)(neighbors[i]>>8);
            buf[_offset + 2] = (u_char)(neighbors[i]>>16);
            buf[_offset + 3] = (u_char)(neighbors[i]>>24);
            // if (i == 0) printf("%x %x %x %x\n", buf[_offset], buf[_offset + 1], buf[_offset + 2], buf[_offset + 3]);
#endif
    }
    _bm->write_block(buf, block_id, _BLOCK_SIZE);
    delete[] buf;
}

void ex_graph::_check_mis() {
    ui cnt = 0;
    bool maximal = true;
    ui *neighbors = new ui[_NODES_PER_BLOCK];
    for (ui i = 1; i <= _n; ++ i) {
        if (_status[i] == _MIS) {
            cnt ++;
            for (ui block_id = _offset[i]; block_id < _offset[i + 1]; ++ block_id) {
                _read_neighbors(block_id, neighbors);
                for (ui j = 0; j < _NODES_PER_BLOCK; ++ j) {
                    if (neighbors[j] == 0) { block_id = _offset[i + 1]; break; }
                    if (_status[neighbors[j]] == _MIS) {
                        cout << "CHECK_IS: WA two adjacent vertices " 
                                << i << ", " << neighbors[j] << " in _MIS.\n";
                        exit(1);
                    }
                }
            }
        } else if (_status[i] == _NOMIS) {
            ui mis_cnt = 0;
            bool find  = false;
            for (ui block_id = _offset[i]; block_id < _offset[i + 1]; ++ block_id) {
                _read_neighbors(block_id, neighbors);
                for (ui j = 0; j < _NODES_PER_BLOCK; ++ j) {
                    if (neighbors[j] == 0) { block_id = _offset[i + 1]; break; }
                    if (_status[neighbors[j]] == _MIS) {
                        find = true;
                        mis_cnt ++;
                    }
                }
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
    delete[] neighbors;
    cout << "|_MIS| " << cnt << ", " << _mis << endl;
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
        << "|2-swappable|: " << swap_cnt_2 << ", after swap |_MIS|: " << _mis << endl;
}

void ex_graph::experiment(const char *_inst_path) {
    string inst_path = string(_inst_path);
    vector<update_t> updates;
    ifstream infile((_INST_PATH + inst_path).c_str());
    if (!infile.is_open()) {
        cout << "instruction file: " << _inst_path << " open failed.\n";
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

    cout << "Update time: " << _utime << "us, |_MIS|: " << _mis << ", |I/O|: " << _io_cnt << endl;
#ifndef NDEBUG
    _check_mis();
    // check_swap();
#endif
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
            ui *neighbors = new ui[_NODES_PER_BLOCK];
            for (ui block_id = _offset[u]; block_id < _offset[u + 1]; ++ block_id) {
                _read_neighbors(block_id, neighbors);
                for (ui j = 0; j < _NODES_PER_BLOCK; ++ j) {
                    ui v = neighbors[j];
                    if (v == 0) { block_id = _offset[u + 1]; break; }
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
            }
      
            for (ui i = 1; i <= _n; ++ i) {
                if (_status[i] == _SWAPPABLE) {
                    _status[i] = _NOMIS;
                    if (_counter[i] == 1) {
                        ui w = 0;
                        for (ui block_id = _offset[i]; block_id < _offset[i + 1]; ++ block_id) {
                            _read_neighbors(block_id, neighbors);
                            for (ui j = 0; j < _NODES_PER_BLOCK; ++ j) {
                                if (neighbors[j] == 0) { block_id = _offset[i + 1]; break; }
                                if (_status[neighbors[j]] == _MIS) {
                                    w = neighbors[j];
                                    block_id = _offset[i + 1];
                                    break;
                                }
                            }
                        }
                        
                        _one_swap(w, true);
                    }
#ifdef _TWO_SWAP_
                    else if (_counter[i] == 2) {
                        ui x = 0, y = 0;
                        for (ui block_id = _offset[i]; block_id < _offset[i + 1]; ++ block_id) {
                            _read_neighbors(block_id, neighbors);
                            for (ui j = 0; j < _NODES_PER_BLOCK; ++ j) {
                                if (neighbors[j] == 0) { block_id = _offset[i + 1]; break; }
                                if (_status[neighbors[j]] == _MIS) {
                                    if (!x) x = neighbors[j];
                                    else {
                                        y = neighbors[j];
                                        block_id = _offset[i + 1];
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
            delete[] neighbors;
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
            ui *neighbors = new ui[_NODES_PER_BLOCK];
            for (ui block_id = _offset[u]; block_id < _offset[u + 1]; ++ block_id) {
                _read_neighbors(block_id, neighbors);
                for (ui j = 0; j < _NODES_PER_BLOCK; ++ j) {
                    ui v = neighbors[j];
                    if (v == 0) { block_id = _offset[u + 1]; break; }
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

            for (ui i = 1; i <= _n; ++ i) {
                if (_status[i] == _SWAPPABLE) {
                    _status[i] = _NOMIS;
                    if (_counter[i] == 1) {
                        ui w = 0;
                        for (ui block_id = _offset[i]; block_id < _offset[i + 1]; ++ block_id) {
                            _read_neighbors(block_id, neighbors);
                            for (ui j = 0; j < _NODES_PER_BLOCK; ++ j) {
                                if (neighbors[j] == 0) { block_id = _offset[i + 1]; break; }
                                if (_status[neighbors[j]] == _MIS) {
                                    w = neighbors[j];
                                    block_id = _offset[i + 1];
                                    break;
                                }
                            }
                        }
                        
                        _one_swap(w, true);
                    }
#ifdef _TWO_SWAP_
                    else if (_counter[i] == 2) {
                        ui x = 0, y = 0;
                        for (ui block_id = _offset[i]; block_id < _offset[i + 1]; ++ block_id) {
                            _read_neighbors(block_id, neighbors);
                            for (ui j = 0; j < _NODES_PER_BLOCK; ++ j) {
                                if (neighbors[j] == 0) { block_id = _offset[i + 1]; break; }
                                if (_status[neighbors[j]] == _MIS) {
                                    if (!x) x = neighbors[j];
                                    else {
                                        y = neighbors[j];
                                        block_id = _offset[i + 1];
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
    if (_status[u] == _MIS && _status[v] == _NOMIS) {
        _counter[v] --;
        if (_counter[v] == 0) {
            _add_into_IS(v);
            _mis ++;
        } else if (_counter[v] == 1) {
            ui w = 0;
            ui *neighbors = new ui[_NODES_PER_BLOCK];
            for (ui block_id = _offset[v]; block_id < _offset[v + 1]; ++ block_id) {
                _read_neighbors(block_id, neighbors);
                for (ui j = 0; j < _NODES_PER_BLOCK; ++ j) {
                    if (neighbors[j] == 0) { block_id = _offset[v + 1]; break; }
                    if (_status[neighbors[j]] == _MIS) {
                        w = neighbors[j];
                        block_id = _offset[v + 1];
                        break;
                    }
                }
            }

            // cout << "w " << w << endl;
            delete[] neighbors;
            _one_swap(w, true);
        }
#ifdef _TWO_SWAP_
        else if (_counter[v] == 2) {
            ui x = 0, y = 0;
            ui *neighbors = new ui[_NODES_PER_BLOCK];
            for (ui block_id = _offset[v]; block_id < _offset[v + 1]; ++ block_id) {
                _read_neighbors(block_id, neighbors);
                for (ui j = 0; j < _NODES_PER_BLOCK; ++ j) {
                    if (neighbors[j] == 0) { block_id = _offset[v + 1]; break; }
                    if (neighbors[j] != 0 && _status[neighbors[j]] == _MIS) {
                        if (!x) x = neighbors[j];
                        else {
                            y = neighbors[j];
                            block_id = _offset[v + 1];
                            break;
                        }
                    }                    
                }
            }
            
            delete[] neighbors;
            _two_swap(x, y);
        }
#endif
    } else {
        if (_counter[u] == 1 && _counter[v] == 1) {
            ui w = 0;
            ui *neighbors = new ui[_NODES_PER_BLOCK];
            for (ui block_id = _offset[v]; block_id < _offset[v + 1]; ++ block_id) {
                _read_neighbors(block_id, neighbors);
                for (ui j = 0; j < _NODES_PER_BLOCK; ++ j) {
                    if (neighbors[j] == 0) { block_id = _offset[v + 1]; break; }
                    if (_status[neighbors[j]] == _MIS) {
                        w = neighbors[j];
                        block_id = _offset[v + 1];
                        break;
                    }
                }
            }
            
            // cout << "w " << w << endl;
            delete[] neighbors;
            _one_swap(w, true);
        }
#ifdef _TWO_SWAP_
        else if (max(_counter[u], _counter[v]) == 2) {
            if (_counter[v] < 2) swap(u, v);
            ui x = 0, y = 0;
            ui *neighbors = new ui[_NODES_PER_BLOCK];
            for (ui block_id = _offset[v]; block_id < _offset[v + 1]; ++ block_id) {
                _read_neighbors(block_id, neighbors);
                for (ui j = 0; j < _NODES_PER_BLOCK; ++ j) {
                    if (neighbors[j] == 0) { block_id = _offset[v + 1]; break; }
                    if (_status[neighbors[j]] == _MIS) {
                        if (!x) x = neighbors[j];
                        else {
                            y = neighbors[j];
                            block_id = _offset[v + 1];
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
    ui *neighbors = new ui[_NODES_PER_BLOCK];
    for (ui block_id = _offset[u]; block_id < _offset[u + 1]; ++ block_id) {
        _read_neighbors(block_id, neighbors);
        for (ui j = 0; j < _NODES_PER_BLOCK; ++ j) {
            ui v = neighbors[j];
            if (v == 0) { block_id = _offset[u + 1]; break; }
            if (_status[v] == _CANDIDATE)
                _counter[v] = 1;
            else if (_status[v] != _DELETED)
                _counter[v] ++;
            if (_status[v] != _AFFECTED && _status[v] != _DELETED)
                _status[v] = _NOMIS;
        }
    }
    delete[] neighbors;
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
#ifndef NDEBUG
    if (u <= 0 || u > _n || v <= 0 || v > _n) {
        cout << "vertices " << u << " or " << v << " does not exist.\n";
        return;
    }
    if (_status[u] == _DELETED || _status[v] == _DELETED) {
        cout << "vertices " << u << " or " << v << " is deleted.\n";
        return;
    }
#endif
    // m ++;
    ui block_id = _offset[u + 1] - 1;
    ui *neighbors = new ui[_NODES_PER_BLOCK];
    for ( ; block_id >= _offset[u]; -- block_id) {
        _read_neighbors(block_id, neighbors);
        if (neighbors[0] != 0) break;
    }
    block_id = max(_offset[u], block_id);
    if (neighbors[_NODES_PER_BLOCK - 1] != 0) block_id = min(block_id + 1, _offset[u + 1] - 1);
    for (ui j = 0; j < _NODES_PER_BLOCK; ++ j) {
        if (neighbors[j] == 0) {
            neighbors[j] = v;
            break;
        }
    }
    _write_neighbors(block_id, neighbors);

    block_id = _offset[v + 1] - 1;
    for ( ; block_id >= _offset[v]; -- block_id) {
        _read_neighbors(block_id, neighbors);
        if (neighbors[0] != 0) break;
    }
    block_id = max(_offset[v], block_id);
    if (neighbors[_NODES_PER_BLOCK - 1] != 0) block_id = min(block_id + 1, _offset[v + 1] - 1);
    for (ui j = 0; j < _NODES_PER_BLOCK; ++ j) {
        if (neighbors[j] == 0) {
            neighbors[j] = u;
            break;
        }
    }
    _write_neighbors(block_id, neighbors);
    delete[] neighbors;
}

void ex_graph::_delete_edge(const ui& u, const ui& v) {
#ifndef NDEBUG
    if (u <= 0 || u > _n || v <= 0 || v > _n) {
        cout << "vertices " << u << " or " << v << " does not eixst.\n";
        return;
    }
    if (_status[u] == _DELETED || _status[v] == _DELETED) {
        cout << "edge (" << u << ", " << v << ") does not exist.\n";
        return;
    }
#endif
    // m --;

    ui block_id = _offset[u + 1] - 1;
    ui *neighbors = new ui[_NODES_PER_BLOCK];
    for (; block_id >= _offset[u]; -- block_id) {
        _read_neighbors(block_id, neighbors);
        if (neighbors[0] != 0) break;
    }
    ui tmp = 0;
    for (ui j = 1; j < _NODES_PER_BLOCK; ++ j) {
        if (neighbors[j] == 0) {
            swap(tmp, neighbors[j - 1]);
            _write_neighbors(block_id, neighbors);
            break;
        }
    }
    if (tmp != v) {
        bool find = false;
        for (block_id = _offset[u]; block_id < _offset[u + 1] && !find; ++ block_id) {
            _read_neighbors(block_id, neighbors);
            for (ui j = 0; j < _NODES_PER_BLOCK; ++j) {
                if (neighbors[j] == v) {
                    neighbors[j] = tmp;
                    find = true;
                    break;
                }
            }
        }
        if (find) _write_neighbors(-- block_id, neighbors);
    }

    block_id = _offset[v + 1] - 1;
    for (; block_id >= _offset[v]; -- block_id) {
        _read_neighbors(block_id, neighbors);
        if (neighbors[0] != 0) break;
    }
    tmp = 0;
    for (ui j = 1; j < _NODES_PER_BLOCK; ++ j) {
        if (neighbors[j] == 0) {
            swap(tmp, neighbors[j - 1]);
            _write_neighbors(block_id, neighbors);
            break;
        }
    }
    // cout << tmp << endl;
    if (tmp != u) {
        bool find = false;
        for (block_id = _offset[v]; block_id < _offset[v + 1] && !find; ++ block_id) {
            _read_neighbors(block_id, neighbors);
            for (ui j = 0; j < _NODES_PER_BLOCK; ++j) {
                if (neighbors[j] == u) {
                    neighbors[j] = tmp;
                    find = true;
                    break;
                }
            }
        }
        if (find) _write_neighbors(-- block_id, neighbors);
    }
    delete[] neighbors;
}

ui ex_graph::_one_swap(const ui& u, bool force) {
#ifndef NDEBUG
    cout << "one swap " << u << endl;
#endif
    ui *neighbors = new ui[_NODES_PER_BLOCK];
    ui cnt = 0, mis_cnt = 0;
    for (ui block_id = _offset[u]; block_id < _offset[u + 1]; ++ block_id) {
        _read_neighbors(block_id, neighbors);
        for (ui j = 0; j < _NODES_PER_BLOCK; ++ j) {
            ui v = neighbors[j];
            if (v == 0) { block_id = _offset[u + 1]; break; }
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
            for (ui block_id = _offset[i]; block_id < _offset[i + 1]; ++ block_id) {
                _read_neighbors(block_id, neighbors);
                for (ui j = 0; j < _NODES_PER_BLOCK; ++ j) {
                    if (neighbors[j] == 0) { block_id = _offset[i + 1]; break; }
                    if (_status[neighbors[j]] == _CANDIDATE)
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
    delete[] neighbors;
    return mis_cnt;
}

ui ex_graph::_two_swap(const ui& u, ui& v) {
#ifndef NDEBUG
    cout << "two swap " << u << ", " << v << endl;
#endif
    ui *neighbors = new ui[_NODES_PER_BLOCK];
    ui cnt = 0, mis_cnt = 0;
    for (ui block_id = _offset[u]; block_id < _offset[u + 1]; ++ block_id) {
        _read_neighbors(block_id, neighbors);
        for (ui j = 0; j < _NODES_PER_BLOCK; ++ j) {
            ui x = neighbors[j];
            if (x == 0) { block_id = _offset[u + 1]; break; }
            if (_status[x] == _NOMIS || _status[x] == _SWAPPABLE) {
                _status[x] = _NOMIS;
                if (_counter[x] == 2) { // find another IS-neighbor of x except u
                    ui y = 0;
                    ui *neighbors_x = new ui[_NODES_PER_BLOCK];
                    for (ui block_id_x = _offset[x]; block_id_x < _offset[x + 1]; ++ block_id_x) {
                        _read_neighbors(block_id_x, neighbors_x);
                        for (ui k = 0; k < _NODES_PER_BLOCK; ++ k) {
                            if (neighbors_x[k] == 0) { block_id_x = _offset[x + 1]; break; }
                            if (neighbors_x[k] != u) {
                                if (_status[neighbors_x[k]] == _MIS) {
                                    y = neighbors_x[k];
                                    _counter[y] ++;
                                    block_id_x = _offset[x + 1];
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

    for (ui block_id = _offset[u]; block_id < _offset[u + 1]; ++ block_id) {
        _read_neighbors(block_id, neighbors);
        for (ui j = 0; j < _NODES_PER_BLOCK; ++ j) {
            if (neighbors[j] == 0) { block_id = _offset[u + 1]; break; }
            if (_status[neighbors[j]] == _NOMIS || _status[neighbors[j]] == _SWAPPABLE) {
                _status[neighbors[j]] = _AFFECTED;
                backup[neighbors[j]] = 1;
                if (_counter[neighbors[j]] == 1) {
                    _status[neighbors[j]] = _CANDIDATE;
                    // backup[neighbors[j]] = 1;
                    cnt ++;
                }
            }
        }
    }

    for (ui block_id = _offset[v]; block_id < _offset[v + 1]; ++ block_id) {
        _read_neighbors(block_id, neighbors);
        for (ui j = 0; j < _NODES_PER_BLOCK; ++ j) {
            if (neighbors[j] == 0) { block_id = _offset[v + 1]; break; }
            if (_status[neighbors[j]] == _NOMIS || _status[neighbors[j]] == _SWAPPABLE) {
                _status[neighbors[j]] = _AFFECTED;
                backup[neighbors[j]] = 1;
                if (_counter[neighbors[j]] == 1) {
                    _status[neighbors[j]] = _CANDIDATE;
                    // backup[neighbors[j]] = 1;
                    cnt ++;
                }
            } else if (_status[neighbors[j]] == _AFFECTED) {
                backup[neighbors[j]] += 1;
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
                for (ui block_id = _offset[i]; block_id < _offset[i + 1]; ++ block_id) {
                    _read_neighbors(block_id, neighbors);
                    for (ui j = 0; j < _NODES_PER_BLOCK; ++ j) {
                        if (neighbors[j] == 0) { block_id = _offset[i + 1]; break; }
                        if (_status[neighbors[j]] == _CANDIDATE)
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
    delete[] neighbors;
    return mis_cnt;
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
