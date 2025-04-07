#include <array>
#include <iostream>
#include <iterator>
#include <sys/time.h>    
#include "track_manager.h"

static TrackManager tm;

#define NUM_BLOCKS_RECORD 5
#define NUM_BLOCKS_OVERDUB 7

bool AreBlocksMatching(const DataBlock &expected, const DataBlock &test) {
  for (uint32_t i = 0; i < expected.samples_.size(); i++) {
    if (expected.samples_[i] != test.samples_[i]) return false;
  }
  return true;
}

void Test_Record_SingleTrack(TrackManager &tm) {
  std::cout << "** test_track_manager.cpp: Test_Record **" << std::endl;
  float value = 0.0f;
  float init_value = 1.1f;
  DataBlock test_data_1p1(init_value);
  DataBlock test_data_incr(value);
  bool result = false;
  uint32_t data_block_number = 0;
  uint32_t num_blocks = NUM_BLOCKS_RECORD + 1;
  uint32_t track_number = 0;

  for (uint32_t index = 0; index < test_data_incr.samples_.size(); index++) {
    test_data_incr.samples_[index] = value++;
  }

  // Starting from off, test event handler for Recording
  // -> first test - insert first block of data use test_data_1p1
  struct timeval t1, t2;
  gettimeofday(&t1, NULL);

  std::cout << "   Set track " << track_number << " to recording and copy data block" << std::endl;
  tm.HandleStateChange_Recording(track_number, test_data_1p1);
  tm.IndexUpdateAllStatesNoChange();
  tm.IndexUpdateRecordNoChange(track_number);

  gettimeofday(&t2, NULL);
  std::cout << "   start recording and update indexes time: " << t2.tv_sec - t1.tv_sec << "s, " << t2.tv_usec - t1.tv_usec << "us" << std::endl;

  // Verify state changed to recording
  result = tm.tracks.at(track_number).IsTrackInRecord();
  if (!result) {
    std::cout << "error: track not in recording state" << std::endl;
  }

  // -> verify manually data and state - get data via mixdown - mixdown is currentIndex
  // mixdown should have been automatically performed
  result = AreBlocksMatching(tm.mixdown, test_data_1p1);
  if (!result) {
    std::cout << "error: test_data_1p1 not matched! " << std::endl;
  }

  // -> second test - insert 5 blocks of data use test_data_incr
  std::cout << "   insert more data blocks " << std::endl;
  for(data_block_number = 1; data_block_number < num_blocks; data_block_number++) {
    tm.HandleStateChange_Recording(track_number, test_data_incr);
    tm.IndexUpdateAllStatesNoChange();
    tm.IndexUpdateRecordNoChange(track_number);
  }

  // -> verify manually and state - get data via mixdown - currentIndex should be 5
  if (data_block_number != tm.tracks.at(track_number).GetCurrentIndex()) {
    std::cout << "error: curr index " << tm.tracks.at(track_number).GetCurrentIndex() << " not exp val " << data_block_number << " **" << std::endl;
  }

  if (data_block_number != tm.tracks.at(track_number).GetEndIndex()) {
    std::cout << "error: end index " << tm.tracks.at(track_number).GetEndIndex() << " not exp val " << data_block_number << " **" << std::endl;
  }

  std::cout << std::endl << "    Performing check on each block **" << std::endl;
  // starting from 1 (as first test is block 0)
  for (data_block_number = 1; data_block_number < num_blocks; data_block_number++) {
    tm.tracks.at(track_number).SetCurrentIndex(data_block_number);
    tm.SetMasterCurrentIndex(data_block_number); // so mixdown uses correct block
    tm.PerformMixdown();
    result = AreBlocksMatching(tm.mixdown, test_data_incr);
    if (!result) {
      std::cout << "error: test_data_incr at " << data_block_number << " not matched!" << result << std::endl;
#ifdef DTEST_VERBOSE
      tm.mixdown.PrintBlock(); 
#endif
    }
    tm.IndexUpdateAllStatesNoChange();
  }
}

// Use after first Record_SingleTrack for simplicity
void Test_Overdub_SingleTrack(TrackManager &tm) {
  std::cout << std::endl << "** test_track_manager.cpp: Test_Overdub **" << std::endl;
  float value = 0.0f;
  float init_value = 1.1f;
  DataBlock test_data_1p1(init_value);
  DataBlock test_data_incr(value);
  DataBlock test_result(value);
  bool result = false;
  uint32_t data_block_number = 0;
  uint32_t num_blocks = NUM_BLOCKS_OVERDUB + 1;
  uint32_t track_number = 0;

  for (uint32_t index = 0; index < test_data_incr.samples_.size(); index++) {
    test_data_incr.samples_[index] = value++;
  }

  // Move indexes to the start, 0
  tm.SetMasterCurrentIndex(data_block_number);
  // zero out the data in the first block and set the track's StartIndex to 2
  tm.tracks.at(track_number).SetStartIndex(2);
  tm.tracks.at(track_number).SetBlockDataToSameValue(0, 0.0f);


#ifdef DTEST_VERBOSE_EXTRA
  tm.PerformMixdown();
  std::cout << " Block 0 at start of test " << std::endl;
  tm.mixdown.PrintBlock();
  std::cout << " Overdubbing with " << std::endl;
  test_data_incr.PrintBlock();
#endif

  // Starting from Record, test event handler for Overdubbing
  // -> first test - insert first block of data use test_data_incr
  struct timeval t1, t2;
  gettimeofday(&t1, NULL);

  tm.HandleStateChange_Overdubbing(track_number, test_data_incr);
  tm.IndexUpdateAllStatesNoChange();
  tm.IndexUpdateOverdubExit(track_number);

  gettimeofday(&t2, NULL);
  std::cout << "   time for overdubbing and updating indexes " << t2.tv_sec - t1.tv_sec << "s, " << t2.tv_usec - t1.tv_usec << "us" << std::endl;

  // Verify state changed to overdubbing
  result = tm.tracks.at(track_number).IsTrackOverdubbing();
  if (!result) {
    std::cout << "error: track not overdubbing " << std::endl;
  }

  // Verify StartIndex set to 0 
  result = tm.tracks.at(track_number).GetStartIndex() == 0;
  if (!result) {
    std::cout << "error: track start index not zero " << std::endl;
  }

  // Create expected block
  MixBlocks(test_data_1p1, test_data_incr, test_result);

  // -> verify manually data and state - get data via mixdown - mixdown is currentIndex
  // mixdown should have been automatically performed
  result = AreBlocksMatching(tm.mixdown, test_data_incr);
  if (!result) {
    std::cout << "error: incremental data not matching " << std::endl;
  }
#ifdef DTEST_VERBOSE_EXTRA
  if (!result) {
    std::cout << " mixdown " << std::endl;
    tm.mixdown.PrintBlock();
    std::cout << " expected " << std::endl;
    test_result.PrintBlock();
  }
#endif

  // -> second test - insert 7 blocks of data use test_data_1p1
  std::cout << "    insert 7 blocks of data, 1p1" << std::endl;
  for(data_block_number = 1; data_block_number < num_blocks; data_block_number++) {
    tm.HandleStateChange_Overdubbing(track_number, test_data_1p1);
    tm.IndexUpdateAllStatesNoChange();
    tm.IndexUpdateOverdubExit(track_number);
  }

  // Set track to Playback because we want the OnExitState to be called

  // -> verify manually and state - get data via mixdown - currentIndex should be 7
  if (data_block_number != tm.tracks.at(track_number).GetCurrentIndex()) {
    std::cout << "error: curr index " << tm.tracks.at(track_number).GetCurrentIndex() << " not exp val " << data_block_number << " **" << std::endl;
  }

  if (data_block_number != tm.tracks.at(track_number).GetEndIndex()) {
    std::cout << "error end index " << tm.tracks.at(track_number).GetEndIndex() << " not exp val " << data_block_number << " **" << std::endl;
  }

  if (data_block_number != tm.GetMasterEndIndex()) {
    std::cout << "error master end index " << tm.GetMasterEndIndex() << " not exp val " << data_block_number << " **" << std::endl;
  }

  std::cout << std::endl << "    Performing check on each block **" << std::endl;
  // starting from 1 (as first test is block 0)
  for (data_block_number = 1; data_block_number < num_blocks; data_block_number++) {
    tm.tracks.at(track_number).SetCurrentIndex(data_block_number);
    tm.SetMasterCurrentIndex(data_block_number); // so mixdown uses correct block
    tm.PerformMixdown();
    if (data_block_number > NUM_BLOCKS_RECORD) {
      result = AreBlocksMatching(tm.mixdown, test_data_1p1);
    } else {
      result = AreBlocksMatching(tm.mixdown, test_result);
    }
    if (!result) {
      std::cout << "error: test_result at " << data_block_number << " not matched! " << std::endl;
      tm.mixdown.PrintBlock(); 
    }
  }
}

void Test_Playback_SingleTrack(TrackManager &tm) {
  std::cout << std::endl << "** test_track_manager.cpp: Test_Playback_SingleTrack **" << std::endl;
  bool result = false;
  float value = 0.0f;
  uint32_t data_block_number = 2;
  uint32_t data_block_number_end = 5;
  uint32_t track_number = 0;
  DataBlock test_result(1.1f);
  DataBlock test_zero(0.0f);

  for (uint32_t index = 0; index < test_result.samples_.size(); index++) {
    test_result.samples_[index] += value++;
  }

  // zero out first two blocks of data and blocks after data_block_number_end
  tm.tracks.at(track_number).SetBlockDataToSameValue(0, 0.0f);
  tm.tracks.at(track_number).SetBlockDataToSameValue(1, 0.0f);
  tm.tracks.at(track_number).SetBlockDataToSameValue(6, 0.0f);
  tm.tracks.at(track_number).SetBlockDataToSameValue(7, 0.0f);


  // modify range to ensure mixdown only follows blocks within start and end for the track
  tm.tracks.at(track_number).SetStartIndex(data_block_number);
  tm.tracks.at(track_number).SetEndIndex(data_block_number_end);
  tm.SetMasterCurrentIndex(0);

  for (uint32_t block_count = 0; block_count < NUM_BLOCKS_OVERDUB + 1; block_count++) { 
    tm.HandleStateChange_Playback(track_number);

    // Verify state changed to recording
    result = tm.tracks.at(track_number).IsTrackInPlayback();
    if (!result) {
      std::cout << "error: track not in playback!" << std::endl;
      std::cout << "    m_c_i " << tm.GetMasterCurrentIndex() << std::endl;
      std::cout << "    t_c_i " << tm.tracks.at(track_number).GetCurrentIndex() << std::endl;
    }
    // Verify data is 0 for blocks 0,1,6,7, rest should be test_result
    if (block_count >= data_block_number && block_count <= data_block_number_end) {
      result = AreBlocksMatching(tm.mixdown, test_result);
    } else {
      result = AreBlocksMatching(tm.mixdown, test_zero);
    }
    if (!result) {
      std::cout << "error: block " << block_count << " no match!" << std::endl;
    }
    tm.IndexUpdateAllStatesNoChange();
    tm.IndexUpdatePlaybackExit(track_number);
  }
}

void Test_PlaybackRepeat_SingleTrack(TrackManager &tm) {
  std::cout << std::endl << "** test_track_manager.cpp: Test_PlaybackRepeat_SingleTrack **" << std::endl;
  bool result = false;
  float value = 0.0f;
  uint32_t data_block_number = 3;
  uint32_t data_block_number_end = 4;
  uint32_t track_number = 0;
  DataBlock test_result(1.1f);

  for (uint32_t index = 0; index < test_result.samples_.size(); index++) {
    test_result.samples_[index] += value++;
  }

  // zero out first two blocks of data and blocks after data_block_number_end
  // already done from Playback test

  // modify range to ensure mixdown only follows blocks within start and end for the track
  tm.tracks.at(track_number).SetStartIndex(data_block_number);
  tm.tracks.at(track_number).SetEndIndex(data_block_number_end);
  tm.SetMasterCurrentIndex(0);

  for (uint32_t block_count = 0; block_count < NUM_BLOCKS_OVERDUB + 1; block_count++) { 
    tm.HandleStateChange_PlaybackRepeat(track_number);

    // Verify state changed to recording
    result = tm.tracks.at(track_number).IsTrackInPlaybackRepeat();
    if (!result) {
      std::cout << "error: track not in playbackRepeat" << std::endl;
      std::cout << "    m_c_i " << tm.GetMasterCurrentIndex() << std::endl;
      std::cout << "    t_c_i " << tm.tracks.at(track_number).GetCurrentIndex() << std::endl;
    }
    // Verify data is test_result for all blocks
    result = AreBlocksMatching(tm.mixdown, test_result);
    if (!result) {
      std::cout << "error: block " << block_count << " no match" << std::endl;
    }
    tm.IndexUpdateAllStatesNoChange();
    tm.IndexUpdateRepeatExit(track_number);
  }
}

void Test_Playback_DualTrack_SecondTrackNoData(TrackManager &tm) {
  std::cout << std::endl << std::endl << "** test_track_manager.cpp: Test_Playback_DualTrack_SecondTrackNoData **" << std::endl;
  bool result = false;
  uint32_t data_block_number = 2;
  uint32_t data_block_number_end = 5;
  uint32_t track_number = 0;
  uint32_t track_number_test = 2;
  float value = 0.0f;
  DataBlock test_zero(0.0f);
  DataBlock test_result(1.1f);

  for (uint32_t index = 0; index < test_result.samples_.size(); index++) {
    test_result.samples_[index] += value++;
  }


  // modify range to ensure mixdown only follows blocks within start and end for the track
  tm.tracks.at(track_number).SetStartIndex(data_block_number);
  tm.tracks.at(track_number).SetEndIndex(data_block_number_end);
  tm.SetMasterCurrentIndex(0);

  // Set track 0 to playback - mixdown relies on master_current
  tm.HandleStateChange_Playback(track_number);
  tm.IndexUpdateAllStatesNoChange();
  tm.IndexUpdatePlaybackExit(track_number);
  tm.HandleStateChange_Playback(track_number_test);

  std::cout << "    entering loop for blocks 1+ " << std::endl;

  for (uint32_t block_count = 1; block_count < NUM_BLOCKS_OVERDUB + 1; block_count++) { 
    tm.HandleStateChange_Playback(track_number_test);

    // Verify state changed to recording
    result = tm.tracks.at(track_number).IsTrackInPlayback();
    if (!result) {
      std::cout << "error: track " << track_number << " not in playback" << std::endl;
    }
    result = tm.tracks.at(track_number_test).IsTrackInPlayback();
    if (!result) {
      std::cout << "error: track " << track_number_test << " not in playback" << std::endl;
      std::cout << "    m_c_i " << tm.GetMasterCurrentIndex() << std::endl;
      std::cout << "    t_" << track_number << "_c_i " << tm.tracks.at(track_number).GetCurrentIndex() << std::endl;
      std::cout << "    t_" << track_number_test << "_c_i " << tm.tracks.at(track_number_test).GetCurrentIndex() << std::endl;
    }

    // Verify data is 0 for blocks 0,1,6,7, rest should be test_result
    if (block_count >= data_block_number && block_count <= data_block_number_end) {
      result = AreBlocksMatching(tm.mixdown, test_result);
    } else {
      result = AreBlocksMatching(tm.mixdown, test_zero);
    }
    if (!result) {
      std::cout << "error: block " << block_count << " no match" << std::endl;
#ifdef DTEST_VERBOSE
      tm.mixdown.PrintBlock();
#endif
    }
    tm.IndexUpdateAllStatesNoChange();
    tm.IndexUpdatePlaybackExit(track_number_test);
  }
}

void Test_MuteUnmute(TrackManager &tm) {
  std::cout << std::endl << std::endl << "** test_track_manager.cpp: Test_MuteUnmute **" << std::endl;
  uint16_t idx = 0;
  // Set all tracks to Playback
  for (auto &t : tm.tracks) {
    if (idx < 5) {
      t.SetTrackToInPlayback();
    } else if (idx >= 5 && idx <= 10) {
      t.SetTrackToInPlaybackRepeat();
    } else {
      t.SetTrackToOff();
    }
    idx++;
  }

  // Mute some, leave others unmuted
  uint16_t mute_unmute = 0xAA55;
  bool result[16] = {true, false, true, false, true, false, true, false,
                     false, true, false, true, false, true, false, true};
  tm.HandleMuteUnmuteTracks(mute_unmute);
  bool test_passed = true;
  idx = 0;
  for (auto &t : tm.tracks) {
    if (result[idx] != t.IsTrackMuted()) {
      std::cout << "error: muted mismatch at " << idx << std::endl;
      std::cout << " exp " << result[idx] << ", act " << t.IsTrackMuted() << std::endl;
      test_passed = false;
    }
    idx++;
  }
  // Unmute all - should all be restored
  tm.HandleMuteUnmuteTracks(0x0000);
  idx = 0;
  for (auto &t : tm.tracks) {
    if (idx < 5) {
      if (!t.IsTrackInPlayback()) {
        test_passed = false;
        std::cout << "error: restore mismatch at " << idx << std::endl;
      }
    } else if (idx >= 5 && idx <= 10) {
      if (!t.IsTrackInPlaybackRepeat()) {
        test_passed = false;
        std::cout << "error: restore mismatch at " << idx << std::endl;
      }
    } else {
      if (!t.IsTrackOff()) {
        test_passed = false;
        std::cout << "error: restore mismatch at " << idx << std::endl;
      }
    }
    idx++;
  }

  if (test_passed) {
    std::cout << " Test Passed " << std::endl;
  }
}

// Need to:
//   Call OnExit for current track
//   Set current track to Playback
//   Then start recording, OnEntry, of new track
void Test_RecordOnDiffTrack_NoPlaybackFirst(TrackManager &tm) {
  std::cout << "\n\n** test_track_manager.cpp: Test_RecordOnDiffTrack_NoPlaybackFirst **" << std::endl;
  float init_value = 1.1f;
  DataBlock test_data_1p1(init_value);
  bool result = false;
  uint32_t track_number = 0;
  uint32_t track_number_1 = 1;
  uint32_t data_block_number = 2;
  uint32_t data_block_number_end = 5;

  // modify range to ensure mixdown only follows blocks within start and end for the track
  tm.tracks.at(track_number).SetStartIndex(data_block_number);
  tm.tracks.at(track_number).SetEndIndex(data_block_number_end);
  tm.SetMasterCurrentIndex(0);



  struct timeval t1, t2;
  gettimeofday(&t1, NULL);

  tm.HandleStateChange_Recording(track_number, test_data_1p1);
  tm.IndexUpdateAllStatesNoChange();
  tm.IndexUpdateRecordNoChange(track_number);

  gettimeofday(&t2, NULL);
  std::cout << "   record and index update time " << t2.tv_sec - t1.tv_sec << "s, " << t2.tv_usec - t1.tv_usec << "us" << std::endl;

  // Verify state changed to recording
  result = tm.tracks.at(track_number).IsTrackInRecord();
  if (!result) {
    std::cout << "error: track not in record? " << std::endl;
  }

  // -> verify manually data and state - get data via mixdown - mixdown is currentIndex
  // mixdown should have been automatically performed
  result = AreBlocksMatching(tm.mixdown, test_data_1p1);
  if (!result) {
    std::cout << "error: test_data_1p1 not matched? " << std::endl;
  }

  // -> Second test - insert first block of data on track 1 use test_data_1p1
  gettimeofday(&t1, NULL);

  tm.HandleStateChange_Recording(track_number_1, test_data_1p1);
  tm.IndexUpdateAllStatesNoChange();
  tm.IndexUpdateRecordNoChange(track_number_1);

  gettimeofday(&t2, NULL);
  std::cout << "   recording and index update time " << t2.tv_sec - t1.tv_sec << "s, " << t2.tv_usec - t1.tv_usec << "us" << std::endl;

  // Verify state changed to recording
  result = tm.tracks.at(track_number_1).IsTrackInRecord();
  if (!result) {
    std::cout << "error: track " << track_number_1 << " not in record" << std::endl;
  }

  // Verify first track state changed to Playback
  result = tm.tracks.at(track_number).IsTrackInPlayback();
  if (!result) {
    std::cout << "error: track " << track_number << " not in payback" << std::endl;
  }

  // Verify indexes
#ifdef DTEST_VERBOSE
  std::cout << "     track " << track_number << " start index " << tm.tracks.at(track_number).GetStartIndex() << std::endl;
  std::cout << "     track " << track_number << " current index " << tm.tracks.at(track_number).GetCurrentIndex() << std::endl;
  std::cout << "     track " << track_number << " end index " << tm.tracks.at(track_number).GetEndIndex() << std::endl;

  // Verify indexes
  std::cout << "     track " << track_number_1 << " start index " << tm.tracks.at(track_number_1).GetStartIndex() << std::endl;
  std::cout << "     track " << track_number_1 << " current index " << tm.tracks.at(track_number_1).GetCurrentIndex() << std::endl;
  std::cout << "     track " << track_number_1 << " end index " << tm.tracks.at(track_number_1).GetEndIndex() << std::endl;

  std::cout << "     master current index " << tm.GetMasterCurrentIndex() << ", end " << tm.GetMasterEndIndex() << std::endl;
#endif

  /// Repeat while switching tracks
  track_number = 1;
  track_number_1 = 0;

  gettimeofday(&t1, NULL);

  tm.HandleStateChange_Recording(track_number, test_data_1p1);
  tm.IndexUpdateAllStatesNoChange();
  tm.IndexUpdateRecordNoChange(track_number);

  gettimeofday(&t2, NULL);
  std::cout << "   recording and indexes time " << t2.tv_sec - t1.tv_sec << "s, " << t2.tv_usec - t1.tv_usec << "us" << std::endl;

  // Verify state changed to recording
  result = tm.tracks.at(track_number).IsTrackInRecord();
  if (!result) {
    std::cout << "error: track " << track_number << " not in record" << std::endl;
  }

  // -> verify manually data and state - get data via mixdown - mixdown is currentIndex
  // mixdown should have been automatically performed
  result = AreBlocksMatching(tm.mixdown, test_data_1p1);
  if (!result) {
    std::cout << "error: test_data_1p1 not matched" << std::endl;
  }

  // -> Second test - insert first block of data on track 1 use test_data_1p1
  gettimeofday(&t1, NULL);

  tm.HandleStateChange_Recording(track_number_1, test_data_1p1);
  tm.IndexUpdateAllStatesNoChange();
  tm.IndexUpdateRecordNoChange(track_number_1);

  gettimeofday(&t2, NULL);
  std::cout << "   recording and index time " << t2.tv_sec - t1.tv_sec << "s, " << t2.tv_usec - t1.tv_usec << "us" << std::endl;

  // Verify state changed to recording
  result = tm.tracks.at(track_number_1).IsTrackInRecord();
  if (!result) {
    std::cout << "error: track " << track_number_1 << " not in record" << std::endl;
  }

  // Verify first track state changed to Playback
  result = tm.tracks.at(track_number).IsTrackInPlayback();
  if (!result) {
    std::cout << "error: track " << track_number << " not in payback" << std::endl;
  }

#ifdef DTEST_VERBOSE
  // Verify indexes
  std::cout << "     track " << track_number << " start index " << tm.tracks.at(track_number).GetStartIndex() << std::endl;
  std::cout << "     track " << track_number << " current index " << tm.tracks.at(track_number).GetCurrentIndex() << std::endl;
  std::cout << "     track " << track_number << " end index " << tm.tracks.at(track_number).GetEndIndex() << std::endl;

  // Verify indexes
  std::cout << "     track " << track_number_1 << " start index " << tm.tracks.at(track_number_1).GetStartIndex() << std::endl;
  std::cout << "     track " << track_number_1 << " current index " << tm.tracks.at(track_number_1).GetCurrentIndex() << std::endl;
  std::cout << "     track " << track_number_1 << " end index " << tm.tracks.at(track_number_1).GetEndIndex() << std::endl;

  std::cout << "     master current index " << tm.GetMasterCurrentIndex() << ", end " << tm.GetMasterEndIndex() << std::endl;
#endif
}

void Test_Playback_IfTracksWereOffFirst(TrackManager &tm) {
  std::cout << std::endl << std::endl << "** test_track_manager.cpp: Test_Playback_IfTracksWereOffFirst **" << std::endl;
  bool result = false;
  DataBlock test_zero(0.0f);
  DataBlock test_result(1.1f);
  uint32_t block_limit = 10;
  uint32_t block_index = 0;

  std::cout << "T_PB_ITWOF: Load tracks with data" << std::endl;
  // Load all tracks for blocks with data
  for (auto &t : tm.tracks) {
    for (block_index = 0; block_index < block_limit; block_index++) {
      t.SetBlockData(block_index, test_result);
    }
  }

  std::cout << "T_PB_ITWOF: Set all tracks to off" << std::endl;
  // Set all tracks to off
  for (auto &t : tm.tracks) {
    t.SetTrackToOff();
  }

  std::cout << "T_PB_ITWOF: Set all tracks to playback" << std::endl;
  // Set all tracks to playback (force, not using HSC_Playback)
  for (auto &t : tm.tracks) {
    t.SetTrackToInPlayback();
    std::cout << "SI " << t.GetStartIndex() << ", EI " << t.GetEndIndex() << ", CI " << t.GetCurrentIndex() << std::endl;
  }

  std::cout << "T_PB_ITWOF: Loop, SMCI, Mixdown, Check " << std::endl;
  // Try mixdown - expect 0 for each master_current_index
  for (uint32_t m_c_i = 0; m_c_i < block_limit; m_c_i++) {
    tm.SetMasterCurrentIndex(m_c_i);
    tm.PerformMixdown();
    result = AreBlocksMatching(tm.mixdown, test_zero);
    if (!result) { tm.mixdown.PrintBlock(); }
  }
}


int main() {
  std::cout << "** test_track_manager.cpp **" << std::endl;
  Test_Record_SingleTrack(tm);
  Test_Overdub_SingleTrack(tm);
  Test_Playback_SingleTrack(tm);
  Test_PlaybackRepeat_SingleTrack(tm);
  Test_Playback_DualTrack_SecondTrackNoData(tm);
  Test_MuteUnmute(tm);
  Test_RecordOnDiffTrack_NoPlaybackFirst(tm);
  Test_Playback_IfTracksWereOffFirst(tm);


  return 0;
}
