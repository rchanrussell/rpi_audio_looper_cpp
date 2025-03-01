#ifndef MIXER_H
#define MIXER_H

#include <array>
#include <iostream>
#include <iterator>

#include "data_block.h"

void MixBlocks(const DataBlock &block1, const DataBlock &block2, DataBlock &mix_down);
#endif // MIXER_H
