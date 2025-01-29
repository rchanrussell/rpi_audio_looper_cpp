#ifndef DATA_BLOCK_H
#define DATA_BLOCK_H

#include <array>
#include <iostream>
#include <iterator>

// TODO Put in DataBlock header - use #def for diff values
#define SAMPLES_PER_BLOCK 128

// TODO Put in own cpp/h file
class DataBlock {
  public:
  // Member Variables
  //float data[SAMPLES_PER_BLOCK];
  std::array<float, SAMPLES_PER_BLOCK> samples;
  // Member Functions
  DataBlock();
  DataBlock(float init_val);
  void SetData(DataBlock &block);
  void GetDataCopy(DataBlock &block);
  void PrintBlock();
};

#endif // DATA_BLOCK_H
