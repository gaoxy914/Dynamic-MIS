#include "utility.h"

void mem_usage(double& vm_usage, double& resident_set) {
    vm_usage = 0.0;
    resident_set = 0.0;
    ifstream stat_stream("/proc/self/stata", ios_base::in);
    string pid, comm, state, ppid, pgrp, session, tty_nr;
    string tpgid, flags, minflt, cminflt, majflt, cmajflt;
    string utime, stime, cutime, cstime, priority, nice;
    string O, itrealvalue, starttime;
    unsigned long vsize;
    long rss;
    stat_stream >> pid >> comm >> state >> ppid >> pgrp >> session >> tty_nr
        >> tpgid >> flags >> minflt >> cminflt >> majflt >> cmajflt
       >> utime >> stime >> cutime >> cstime >> priority >> nice
        >> O >> itrealvalue >> starttime >> vsize >> rss;
        long page_size_kb = sysconf(_SC_PAGE_SIZE) / 1024;
        vm_usage = vsize / 1024.0;
        resident_set = rss*page_size_kb;
}

void int2char(const u_int& length, u_int *src, u_char *des) {
    for (u_int i = 0; i < length; ++ i) {
        u_int offset = i*sizeof(u_int);
        des[offset] = (u_char)src[i];
        des[offset + 1] = (u_char)(src[i]>>8);
        des[offset + 2] = (u_char)(src[i]>>16);
        des[offset + 3] = (u_char)(src[i]>>24);
        // if (i == 0) printf("%x %x %x %x\n", des[offset], des[offset + 1], des[offset + 2], des[offset + 3]);
    }
}

void char2int(const u_int& length, u_char *src, u_int *des) {
    for (u_int i = 0; i < length; ++ i) {
        u_int offset = i*sizeof(u_int);
        des[i] = (u_int)src[offset] | (u_int)src[offset + 1]<<8 | (u_int)src[offset + 2]<<16 | (u_int)src[offset + 3]<<24;
    }
}


blockmanager::blockmanager(const char *file_name, int file_mode, int block_size) {
    this->block_size = block_size;
    strcpy(this->file_name, file_name);
    this->open_file(this->file_name, file_mode);
}

blockmanager::~blockmanager() {
    this->close_file(this->open_fd);
}

int blockmanager::open_file(const char *file_name, int file_mode) {
    if (this == NULL) {
        cout << "NULL Pointer!\n";
        return FAIL;
    }
    if (file_name == NULL) {
        cout << "File Name is NULL!\n";
        return FAIL;
    }
    if (this->block_size < 0) {
        cout << "block_size is not set yet!\n";
        return FAIL;
    }
    open_fd = -1;
    int exist = access(file_name, F_OK);
    if (exist < 0) {
        if (file_mode == O_CREAT) {
#if _USE_GNU
            open_fd = open(file_name, O_RDWR | O_CREAT | O_EXCL | O_DIRECT, 0666);
#else
            open_fd = open(file_name, O_RDWR | O_CREAT | O_EXCL, 0666);
#endif
        } else {
            cout << "No File is opened!\n";
            return FAIL;
        }
    } else {
#if _USE_GNU
            open_fd = open(file_name, O_RDWR | O_EXCL | O_DIRECT, 0666);
#else
            open_fd = open(file_name, O_RDWR | O_EXCL, 0666);
#endif
    }
    if (open_fd < 0) {
        cout << "Open File Failed!\n";
        return FAIL;
    }
    if (lseek(open_fd, 0, SEEK_SET) == -1) {
        cout << "Can not seek!\n";
        return FAIL;
    }
    if (lockf(open_fd, F_LOCK, 0) == -1) {
        cout << "Failed to lock file " << file_name << endl;
        close(open_fd);
        return FAIL;
    }
    return SUCCESS;
}

int blockmanager::read_block(u_char *des, ulong block_id, int block_length) {
    if (this == NULL) {
        cout << "NULL Pointer!\n";
        return FAIL;
    }
    ulong block_offset = this->get_offset(block_id);
    if (lseek(this->open_fd, block_offset, SEEK_SET) == -1) {
        cout << "Can not seek!\n";
        return FAIL;
    }
#if _USE_GNU
    u_char *alignmentbuf;
    int ret = posix_memalign((void**)&(alignmentbuf), 4096, block_length);
    if (read(this->open_fd, alignmentbuf, block_length) != block_length) {
        cout << "Can not read data.\n";
        return FAIL;
    }
    memmove(des, alignmentbuf, block_length);
    delete[] alignmentbuf;
#else
    if (read(this->open_fd, des, block_length) != block_length) {
        cout << "Can not read data.\n";
        return FAIL;
    }
#endif
    return SUCCESS;
}

int blockmanager::write_block(u_char *src, ulong block_id, int block_length) {
    if (this == NULL) {
        cout << "NULL Pointer.\n";
        return FAIL;
    }
    ulong block_offset = this->get_offset(block_id);
    // cout << "block id " << block_id << " offset " << block_offset << endl;
    if (lseek(this->open_fd, block_offset, SEEK_SET) == -1) {
        cout << "Can not seek!\n";
        return FAIL;
    }
#if _USE_GNU
    u_char *alignmentbuf;
    int ret = posix_memalign((void**)&(alignmentbuf), 4096, block_length);
    // cout << "ret " << ret << endl;
    memmove(alignmentbuf, src, block_length);
    if (write(this->open_fd, alignmentbuf, block_length) != block_length) {
        cout << "Can not write data.\n";
        return FAIL;
    }
    delete[] alignmentbuf;
#else
    if (write(this->open_fd, src, block_length) == -1) {
        cout << "Can not write data.\n";
        return FAIL;
    }
#endif
    return SUCCESS;
}

ulong blockmanager::get_offset(ulong block_id) {
    return block_id*this->block_size;
}

int blockmanager::close_file(int fd) {
    // cout << "close " << fd << endl;
    if (this == NULL) {
        cout << "NULL Pointer.\n";
        return FAIL;
    }
    if (close(fd) == -1) {
        cout << "File Close Failed!\n";
        cout << "Message: " << strerror(errno) << endl;
        return FAIL;
    }
    return SUCCESS;
}
