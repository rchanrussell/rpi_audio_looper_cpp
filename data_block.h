#ifndef DATA_BLOCK_H
#define DATA_BLOCK_H

#include <array>
#include <iostream>
#include <iterator>

#include "util.h"

class DataBlock {
  public:
  // Member Variables
  std::array<float, SAMPLES_PER_BLOCK> samples_;

  // Member Functions
  DataBlock();
  DataBlock(float init_val);
  void SetData(DataBlock &block);
  void GetDataCopy(DataBlock &block);
  void PrintBlock();
};

#endif // DATA_BLOCK_H
