#include "mixer.h"

void MixBlocks(const DataBlock &block1, const DataBlock &block2, DataBlock &mix_down) {
  for (int i = 0; i < block1.samples.size(); i++) {
    // don't try compression or limiting, user can adjust volumes
    mix_down.samples.at(i) += block1.samples.at(i) + block2.samples.at(i);
  }
} 

