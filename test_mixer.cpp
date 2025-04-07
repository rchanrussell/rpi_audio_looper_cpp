#include <array>
#include <iostream>
#include <iterator>
#include "track.h"
#include "mixer.h"

static Track t1, t2, t3, t4;

bool AreBlocksMatching(const DataBlock &expected, const DataBlock &test) {
  for (uint32_t i = 0; i < expected.samples_.size(); i++) {
    if (expected.samples_[i] != test.samples_[i]) return false;
  }
  return true;
}

bool Test_SimpleMixerSummation(void) {
  // Create four tracks, populate with same sample values
  // sum and check against expected values
  DataBlock mixed, expected_results;
  float expected_value = 36.4f;
  float offset = 0.0f;

  for (int j = 0; j < MAX_BLOCK_COUNT; j++) {
    t1.SetBlockDataToSameValue(j, offset + 0.1f);
    t2.SetBlockDataToSameValue(j, offset + 1.1f);
    t3.SetBlockDataToSameValue(j, offset + 2.1f);
    t4.SetBlockDataToSameValue(j, offset + 3.1f);
    offset += 10.0f;
  }

  expected_results.samples_.fill(expected_value);

//  MixBlocks(t1.frame_blocks.at(0), t2.frame_blocks.at(0), mixed);
  MixBlocks(t1.GetBlockData(0), t2.GetBlockData(0), mixed);
  MixBlocks(t3.GetBlockData(1), t4.GetBlockData(2), mixed);

  return AreBlocksMatching(expected_results, mixed);
}

// TODO Turn this into a test
int main() {
  std::cout << "** test_mixer.cpp **" << std::endl;
  bool tests[5] = {false, false, false, false, false};
  tests[0] = Test_SimpleMixerSummation();
  std::cout << tests[0] << std::endl;
 
  return 0;
}
