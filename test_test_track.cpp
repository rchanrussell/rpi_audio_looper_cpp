#include <array>
#include <iostream>
#include <iterator>
#include <sys/time.h>    
#include <cstdlib>
#include <unistd.h>
#include <chrono>
#include <thread>
#include "track_manager.h"

static Track test_track(0.0f);
static TrackManager tm;

bool AreBlocksMatching(const DataBlock &expected, const DataBlock &test) {
  for (unsigned long int i = 0; i < expected.samples_.size(); i++) {
    if (expected.samples_[i] != test.samples_[i]) return false;
  }
  return true;
}

void Test_SimulateRecord(Track &test_track) {
  std::cout << "** test_track.cpp: Test_SimulateRecord **" << std::endl;
  float value = 0.0f;
  float init_value = 1.1f;
  DataBlock test_data_1p1(init_value);
  DataBlock test_data_incr(0.0f);
  bool result = false;

  for (uint32_t index = 0; index < test_data_incr.samples_.size(); index++) {
    test_data_incr.samples_[index] = value++;
  }

  // Starting from scratch, all data is zero
  // Simulate Record -- overwrite

  // Start and Current Index are default to zero and are updated after data transfer
  // SetStartIndex(track.GetCurrentIndex())
  test_track.SetStartIndex(test_track.GetCurrentIndex());
  // SetBlockData(track.GetCurrentIndex(), test_data_incr)
  test_track.SetBlockData(test_track.GetCurrentIndex(), test_data_incr);

  // Verify Data - manually
  result = AreBlocksMatching(test_track.GetBlockData(test_track.GetCurrentIndex()),
                             test_data_incr);
  std::cout << "** test_data_incr matched? " << result << " **" << std::endl;

  // Test Next Block with different data
  // IncrementCurrentIndex()
  test_track.IncrementCurrentIndex();

  // End indext is updated when we exit recording state
  // SetBlockData(track.GetCurrentIndex(), test_data_1p1)

  test_track.SetBlockData(test_track.GetCurrentIndex(), test_data_1p1);

  // Verify Data - manually
  result = AreBlocksMatching(test_track.GetBlockData(test_track.GetCurrentIndex()),
                             test_data_1p1);
  std::cout << "** test_data_1p1 matched? " << result << " **" << std::endl;

  // Ending recording state - SetEndIndex(track.GetCurrentIndex())
  test_track.SetEndIndex(test_track.GetCurrentIndex());
  // Set State Recording
  test_track.SetTrackToInRecord();
}

void Test_SimulatePlayback(Track &test_track) {
  std::cout << "** test_track.cpp: Test_SimulatePlayback **" << std::endl;
  float value = 0.0f;
  float init_value = 1.1f;
  DataBlock test_data_1p1(init_value);
  DataBlock test_data_incr(0.0f);
  bool result = false;

  for (uint32_t index = 0; index < test_data_incr.samples_.size(); index++) {
    test_data_incr.samples_[index] = value++;
  }

  // Starting from a previously set recording state
  result = test_track.IsTrackInRecord();
  std::cout << "** is track recording? " << result << " **" << std::endl;

  // Move current index to start index
  test_track.SetCurrentIndex(test_track.GetStartIndex());
  // Verify Data - manually
  result = AreBlocksMatching(test_track.GetBlockData(test_track.GetCurrentIndex()),
                             test_data_incr);
  std::cout << "** test_data_incr matched? " << result << " **" << std::endl;

  // increment current index
  test_track.IncrementCurrentIndex();
  // Verify Data - manually
  result = AreBlocksMatching(test_track.GetBlockData(test_track.GetCurrentIndex()),
                             test_data_1p1);
  std::cout << "** test_data_1p1 matched? " << result << " **" << std::endl;

  // Set state to Playback
  test_track.SetTrackToInPlayback();
}

void Test_SimulateOverdub(Track &test_track) {
  std::cout << "** test_track.cpp: Test_SimulateOverdub **" << std::endl;
  float value = 0.0f;
  float init_value = 1.1f;
  DataBlock test_data_1p1(init_value), test_data_1p1_result(init_value + 10.0f);
  DataBlock test_data_incr(0.0f);
  DataBlock test_data_10(10.0f);
  DataBlock mixdown(0.0f); // temporary because MixBlocks doesn't modify sources
  bool result = false;

  for (uint32_t index = 0; index < test_data_incr.samples_.size(); index++) {
    test_data_incr.samples_[index] = value++;
  }

  // Test assumes coming from playback test
  // Data is incr then 1p1
  // Simulate Overdub

  // Current Index is at end index, pointing to last block written
  // Verify Data - manually
  result = AreBlocksMatching(test_track.GetBlockData(test_track.GetCurrentIndex()),
                             test_data_1p1);
  std::cout << "** test_data_incr pre-overdub block matched? " << result << " **" << std::endl;

  // We will overdub/mix this last block and write to the next block, extending the end
  // index when we are finished, leaving the first block unchanged
  // MixBlock current block (only two written, so this is test_data_incr)
  struct timeval t1, t2;
  gettimeofday(&t1, NULL);

  MixBlocks(test_track.GetBlockData(test_track.GetCurrentIndex()),
            test_data_10,
            mixdown);
  // Now we update the track
  test_track.SetBlockData(test_track.GetCurrentIndex(), mixdown);

  gettimeofday(&t2, NULL);
  std::cout << t2.tv_sec - t1.tv_sec << "s, " << t2.tv_usec - t1.tv_usec << "us" << std::endl;


  // Verify Data - manually
  result = AreBlocksMatching(test_track.GetBlockData(test_track.GetCurrentIndex()),
                             test_data_1p1_result);
  std::cout << "** test_data_incr_result matched? " << result << " **" << std::endl;

  // Overdub an additional block, next block should be all zeros
  test_track.IncrementCurrentIndex();

  // End indext is updated when we exit recording state
  // SetBlockData(track.GetCurrentIndex(), test_data_1p1)

  test_track.SetBlockData(test_track.GetCurrentIndex(), test_data_1p1);

  // Verify Data - manually
  result = AreBlocksMatching(test_track.GetBlockData(test_track.GetCurrentIndex()),
                             test_data_1p1);
  std::cout << "** test_data_1p1 new block matched? " << result << " **" << std::endl;

  // Ending recording state - SetEndIndex(track.GetCurrentIndex())
  test_track.SetEndIndex(test_track.GetCurrentIndex());
  // Set State Overdubbing
  test_track.SetTrackToOverdubbing();
}



void Test_MixblocksWithTiming() {
  std::cout << "** test_track.cpp: Test_MixblocksWithTiming **" << std::endl;
  TrackManager tm;
  float offset = 0.0f;

  // Intialize Data
  for (auto &t : tm.tracks) {
    for (uint32_t j = 0; j < 4; j++) {
      t.SetBlockDataToSameValue(j, offset + 0.1f);
/*
      t.PrintTrackBlock(j);
      std::cout << std::endl;
*/
    }
    offset += 10.0f;
  }
  struct timeval t1, t2;
  gettimeofday(&t1, NULL);

  for (unsigned long int t = 0; t < tm.tracks.size(); t+=2) {
//    std::cout << "Mixing block 0 tracks " << t << " and " << t + 1 << std::endl;
    MixBlocks(tm.tracks.at(t).GetBlockData(0), tm.tracks.at(t+1).GetBlockData(0),tm.mixdown);
  }

  gettimeofday(&t2, NULL);
  std::cout << t2.tv_sec - t1.tv_sec << "s, " << t2.tv_usec - t1.tv_usec << "us" << std::endl;
#ifdef DTEST_VERBOSE
  tm.mixdown.PrintBlock();
#endif
}

void Test_CreateVoidPtrMemConvertToDataBlock() {
  uint8_t val = 1;
  DataBlock ip(0.0f);
  int *ptr =(int *)calloc(1024, 1);
  if (!ptr) { std::cout << "Calloc failed" << std::endl; return; }

  for (int idx=0; idx<1024; idx++) {
    if (idx > 0 && (idx % 128 == 0)) { val++; }
    ptr[idx] = val;
  }

  // below does not compile
  // ip.samples = (std::array<float, SAMPLES_PER_BLOCK>)*ptr;

  // just return the address of .samples -- does NOT work
  // memcpy(&ip.samples, ptr, SAMPLES_PER_BLOCK);
  struct timeval t1, t2, tdiff;

  gettimeofday(&t1, NULL);
  std::copy(ptr, ptr + SAMPLES_PER_BLOCK, begin(ip.samples_));
  gettimeofday(&t2, NULL);
  timersub(&t2, &t1, &tdiff);
  std::cout << "td: " << tdiff.tv_sec << "," << tdiff.tv_usec << std::endl;

  for (int idx=0; idx <SAMPLES_PER_BLOCK; idx++) {
    if (ptr[idx] != ip.samples_.at(idx)) {
      std::cout << "memcpy ptr -- mismatch at " << idx << std::endl;
      std::cout << "ptr: " << unsigned(ptr[idx]) << ", ip.s[idx]:" << ip.samples_.at(idx) << std::endl;
      break;
    }
  }

  gettimeofday(&t1, NULL);
  for (int idx =0; idx < SAMPLES_PER_BLOCK; idx++) {
    ip.samples_.at(idx) = ptr[idx];
  }
  gettimeofday(&t2, NULL);
  timersub(&t2, &t1, &tdiff);
  std::cout << "td: " << tdiff.tv_sec << "," << tdiff.tv_usec << std::endl;

  for (int idx=0; idx <SAMPLES_PER_BLOCK; idx++) {
    if (ptr[idx] != ip.samples_.at(idx)) {
      std::cout << "for:ip.s[i]=ptr[i] -- mismatch at " << idx << std::endl;
      std::cout << "ptr: " << unsigned(ptr[idx]) << ", ip.s[idx]:" << ip.samples_.at(idx) << std::endl;
      break;
    }
  }

  tm.CopyToInputBuffer(ptr ,128);
  tm.CopyBufferToTrack(0);
  DataBlock db = tm.tracks.at(0).GetBlockData(0);
  tm.tracks.at(1).SetBlockDataToSameValue(0, 2.0f);
  db.PrintBlock();
  db = tm.tracks.at(1).GetBlockData(0);
  db.PrintBlock();
  tm.tracks.at(0).SetEndIndex(1);
  tm.tracks.at(1).SetEndIndex(1);
  tm.SetMasterEndIndex(1);
  tm.SetTrackStatePlayback(0);
  tm.SetTrackStatePlayback(1);
  tm.PerformMixdown();
  tm.mixdown.PrintBlock();

  std::cout << "    copymixdown" << std::endl;
  //int *pm =(int *)calloc(1024,1);
  int pm[1024];
  tm.CopyMixdownToBuffer(pm, 128);
  for (int idx=0; idx<128; idx++) {
    std::cout << pm[idx] << ", ";
    if (idx % 8 == 0) { std::cout << std::endl; }
  }

  free(ptr);
  //free(pm);
}


// TODO Turn this into a test
int main() {
  std::cout << "** test_track.cpp **" << std::endl;
#if 0
  Test_SimulateRecord(test_track);
  Test_SimulatePlayback(test_track);
  Test_SimulateOverdub(test_track);
#endif

  Test_CreateVoidPtrMemConvertToDataBlock();

  return 0;
}
