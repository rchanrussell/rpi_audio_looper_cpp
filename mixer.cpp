#include "mixer.h"

void MixBlocks(const DataBlock &block1, const DataBlock &block2, DataBlock &mix_down) {
  for (uint32_t i = 0; i < block1.samples_.size(); i++) {
    // don't try compression or limiting, user can adjust volumes
    mix_down.samples_.at(i) += block1.samples_.at(i) + block2.samples_.at(i);
  }
} 

