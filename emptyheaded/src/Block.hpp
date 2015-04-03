#ifndef _TRIE_H_
#define _TRIE_H_

#include "set/ops.hpp"

class Block{
  public:
  std::unordered_map<uint32_t,Block*> *map;
  Set<uinteger> *data;

  Block(){
    map = new std::unordered_map<uint32_t,Block*>();
  }
};

#endif