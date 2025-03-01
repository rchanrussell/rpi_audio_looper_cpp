#include "data_block.h"

// Use Delegating Constructors to avoid duplication
DataBlock::DataBlock():DataBlock(0.0f) {
}

DataBlock::DataBlock(float init_val) {
  samples_.fill(init_val);
}

void DataBlock::SetData(DataBlock &block) {
  samples_ = block.samples_;
}

void DataBlock::GetDataCopy(DataBlock &block) {
  block.samples_ = samples_;
}

void DataBlock::PrintBlock() {
  int line_count = 0;
  for (const auto& d : samples_) {
    std::cout << d << ' ';
    line_count++;
    if (line_count % 16 == 0) {
      std::cout << std::endl;
    }
  }
}

