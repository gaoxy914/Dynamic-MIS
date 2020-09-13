#ifndef _UTILITY_H_
#define _UTILITY_H_

#include <string>
#include <vector>
#include <cassert>
#include <iostream>
#include <fstream>
#include <cstdio>
#include <algorithm>
#include <cmath>
#include <queue>
#include <unordered_map>
#include <ios>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <fcntl.h>
#include <stdio.h>
#include <inttypes.h>
#include <semaphore.h>
#include <string.h>
#include <malloc.h>
#include <stdlib.h>

#define _LINUX_
#ifdef _LINUX_
	#include <sys/time.h>
#endif

#define NDEBUG

// #define _TWO_SWAP_

#define _EDGE_ADDITION 0
#define _EDGE_DELETION 1
#define _VERTEX_ADDITION 3
#define _VERTEX_DELETION 2
#define _DEFAULT -1

#define _INST_NUM 1000

#define _BLOCK_SIZE 4096

#define _GRAPH_PATH "Graphdata/"
#define _MIS_PATH "InitialMis/"
#define _INST_PATH "Instruction/"
#define _NGRAPH_PATH "UGraphdata/"
#define _GRAPHDISK_PATH "GraphDisk/"

#define _USE_GNU 1

typedef unsigned long ulong;

using namespace std;

enum ret
{
	SUCCESS = 1,
	FAIL = 0,
};

class blockmanager
{
	int open_fd = -1;
	int block_size = -1;
	char file_name[256];

public:
	blockmanager(){};
	blockmanager(const char *file_name, int file_mode, int block_size);
	~blockmanager();
	int open_file(const char *file_name, int file_mode);
	int read_block(u_char *des, ulong block_id, int block_length);
	int write_block(u_char *src, ulong block_id, int block_length);
	ulong get_offset(ulong block_id);
	int close_file(int fd);
};

#endif
