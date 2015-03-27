#ifndef COMMON_H
#define COMMON_H

#include <x86intrin.h>
#include <unordered_map>
#include <ctime>
#include <sys/time.h>
#include <sys/resource.h>
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <iterator>
#include <algorithm>  // for std::find
#include <cstring>
#include <sys/mman.h>
#include <fcntl.h>    /* For O_RDWR */
#include <unistd.h>   /* For open(), creat() */
#include <math.h>
#include <unistd.h>
#include <tuple>
#include <cstdarg>
#include <set>
#include "assert.h"

//static size_t ADDRESS_BITS_PER_BLOCK = 7;
//static size_t BLOCK_SIZE = 128;
//static double BITSET_THRESHOLD = 1.0 / 16.0;

// Experts only! Proceed wih caution!
//#define ENABLE_PCM
//#define ENABLE_PRINT_THREAD_TIMES
//#define ENABLE_ATOMIC_UNION

//TODO: Replace with new command line arguments.

//Needed for parallelization, prevents false sharing of cache lines
#define PADDING 300
#define MAX_THREADS 512

//#define ATTRIBUTES
#define WRITE_TABLE 0

#define COMPRESSION 0
#define PERFORMANCE 1
#define VECTORIZE 1

// Enables/disables pruning
#define PRUNING
//#define NEW_BITSET

// Enables/disables hybrid that always chooses U-Int
//#define HYBRID_UINT

//#define STATS

//CONSTANTS THAT SHOULD NOT CHANGE
#define SHORTS_PER_REG 8
#define INTS_PER_REG 4
#define BYTES_PER_REG 16

static size_t NUM_THREADS = 1;

namespace type{
  enum file{
    csv = 0,
    tsv = 1,
    binary = 2
  };

  enum primitives{
    uint32_t = 0,
    uint64_t = 1,
    string = 2
  };

  enum layout: uint8_t {
    BITSET = 0,
    PSHORT = 1,
    UINTEGER = 2,
    BITPACKED = 3,
    VARIANT = 4,
    HYBRID = 5,
    KUNLE = 6,
    BITSET_NEW = 7,
    NEW_TYPE = 8
  };

}

#endif
