#include <array>
#include <iostream>
#include <iterator>
#include <sys/time.h>    
#include "track_manager.h"

#define NUM_BLOCKS_RECORD 5
#define NUM_BLOCKS_OVERDUB 7

bool AreBlocksMatching(const DataBlock &expected, const DataBlock &test) {
  for (int i = 0; i < expected.samples.size(); i++) {
    if (expected.samples[i] != test.samples[i]) return false;
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

  for (uint32_t index = 0; index < test_data_incr.samples.size(); index++) {
    test_data_incr.samples[index] = value++;
  }

  // Starting from off, test event handler for Recording
  // -> first test - insert first block of data use test_data_1p1
  struct timeval t1, t2;
  gettimeofday(&t1, NULL);

  tm.HandleStateChange_Recording(track_number, test_data_1p1);
  tm.HandleIndexUpdate_AlreadyInState_AllStates();

  gettimeofday(&t2, NULL);
  std::cout << t2.tv_sec - t1.tv_sec << "s, " << t2.tv_usec - t1.tv_usec << "us" << std::endl;

  // Verify state changed to recording
  result = tm.tracks.at(track_number).IsTrackInRecord();
  std::cout << "** is track in record? " << result << " **" << std::endl;

  // -> verify manually data and state - get data via mixdown - mixdown is currentIndex
  // mixdown should have been automatically performed
  result = AreBlocksMatching(tm.mixdown, test_data_1p1);
  std::cout << "** test_data_1p1 matched? " << result << " **" << std::endl << std::endl << std::endl;

  // -> second test - insert 5 blocks of data use test_data_incr
  for(data_block_number = 1; data_block_number < num_blocks; data_block_number++) {
    tm.HandleStateChange_Recording(track_number, test_data_incr);
    tm.HandleIndexUpdate_AlreadyInState_AllStates();
  }
  data_block_number--; // don't want it set to 6

  // -> verify manually and state - get data via mixdown - currentIndex should be 5
  if (data_block_number != tm.tracks.at(track_number).GetCurrentIndex()) {
  std::cout << "** curr index " << tm.tracks.at(track_number).GetCurrentIndex() << " not exp val " << data_block_number << " **" << std::endl;
  }

  if (data_block_number != tm.tracks.at(track_number).GetEndIndex()) {
    std::cout << "** end index " << tm.tracks.at(track_number).GetEndIndex() << " not exp val " << data_block_number << " **" << std::endl;
  }

  std::cout << std::endl << "  ** Performing check on each block **" << std::endl;
  // starting from 1 (as first test is block 0)
  for (data_block_number = 1; data_block_number < num_blocks; data_block_number++) {
    tm.tracks.at(track_number).SetCurrentIndex(data_block_number);
    tm.SetMasterCurrentIndex(data_block_number); // so mixdown uses correct block
    tm.PerformMixdown();
#ifdef DTEST_VERBOSE
    tm.mixdown.PrintBlock(); 
#endif
    result = AreBlocksMatching(tm.mixdown, test_data_incr);
    std::cout << "** test_data_incr at " << data_block_number << " matched? " << result << " **" << std::endl << std::endl;
    tm.HandleIndexUpdate_AlreadyInState_AllStates();
  }
}

// Use after first Record_SingleTrack for simplicity
void Test_Overdub_SingleTrack(TrackManager &tm) {
  std::cout << "** test_track_manager.cpp: Test_Overdub **" << std::endl;
  float value = 0.0f;
  float init_value = 1.1f;
  DataBlock test_data_1p1(init_value);
  DataBlock test_data_incr(value);
  DataBlock test_result(value);
  bool result = false;
  uint32_t data_block_number = 0;
  uint32_t num_blocks = NUM_BLOCKS_OVERDUB + 1;
  uint32_t track_number = 0;

  for (uint32_t index = 0; index < test_data_incr.samples.size(); index++) {
    test_data_incr.samples[index] = value++;
  }

  // Move indexes to the start, 0
  tm.SetMasterCurrentIndex(data_block_number);
  // zero out the data in the first block and set the track's StartIndex to 2
  tm.tracks.at(track_number).SetStartIndex(2);
  tm.tracks.at(track_number).SetBlockDataToSameValue(0, 0.0f);


#ifdef DTEST_VERBOSE
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
  tm.HandleIndexUpdate_AlreadyInState_AllStates();

  gettimeofday(&t2, NULL);
  std::cout << t2.tv_sec - t1.tv_sec << "s, " << t2.tv_usec - t1.tv_usec << "us" << std::endl;

  // Verify state changed to overdubbing
  result = tm.tracks.at(track_number).IsTrackOverdubbing();
  std::cout << "** is track overdubbing? " << result << " **" << std::endl;

  // Verify StartIndex set to 0 
  result = tm.tracks.at(track_number).GetStartIndex() == 0;
  std::cout << "** is track start index 0? " << result << " **" << std::endl;

  // Create expected block
  MixBlocks(test_data_1p1, test_data_incr, test_result);

  // -> verify manually data and state - get data via mixdown - mixdown is currentIndex
  // mixdown should have been automatically performed
  result = AreBlocksMatching(tm.mixdown, test_data_incr);
  std::cout << "** test_data_incr matched? " << result << " **" << std::endl << std::endl << std::endl;

#ifdef DTEST_VERBOSE
  if (!result) {
    std::cout << " mixdown " << std::endl;
    tm.mixdown.PrintBlock();
    std::cout << " expected " << std::endl;
    test_result.PrintBlock();
  }
#endif

  // -> second test - insert 7 blocks of data use test_data_1p1
  for(data_block_number = 1; data_block_number < num_blocks; data_block_number++) {
    tm.HandleStateChange_Overdubbing(track_number, test_data_1p1);
    tm.HandleIndexUpdate_AlreadyInState_AllStates();
  }

  // Set track to Playback because we want the OnExitState to be called

  // -> verify manually and state - get data via mixdown - currentIndex should be 7
  if (data_block_number != tm.tracks.at(track_number).GetCurrentIndex()) {
  std::cout << "** curr index " << tm.tracks.at(track_number).GetCurrentIndex() << " not exp val " << data_block_number << " **" << std::endl;
  }

  if (data_block_number != tm.tracks.at(track_number).GetEndIndex()) {
    std::cout << "** end index " << tm.tracks.at(track_number).GetEndIndex() << " not exp val " << data_block_number << " **" << std::endl;
  }

  if (data_block_number != tm.GetMasterEndIndex()) {
    std::cout << "** master end index " << tm.GetMasterEndIndex() << " not exp val " << data_block_number << " **" << std::endl;
  }

  std::cout << std::endl << "  ** Performing check on each block **" << std::endl;
  // starting from 1 (as first test is block 0)
  for (data_block_number = 1; data_block_number < num_blocks; data_block_number++) {
    tm.tracks.at(track_number).SetCurrentIndex(data_block_number);
    tm.SetMasterCurrentIndex(data_block_number); // so mixdown uses correct block
    tm.PerformMixdown();
#ifdef DTEST_VERBOSE
    tm.mixdown.PrintBlock(); 
#endif
    if (data_block_number > NUM_BLOCKS_RECORD) {
      result = AreBlocksMatching(tm.mixdown, test_data_1p1);
    } else {
      result = AreBlocksMatching(tm.mixdown, test_result);
    }
    std::cout << "** test_result at " << data_block_number << " matched? " << result << " **" << std::endl << std::endl;
  }
}

void Test_Playback_SingleTrack(TrackManager &tm) {
  std::cout << "** test_track_manager.cpp: Test_Playback_SingleTrack **" << std::endl;
  bool result = false;
  float value = 0.0f;
  uint32_t data_block_number = 2;
  uint32_t data_block_number_end = 5;
  uint32_t track_number = 0;
  DataBlock test_result(1.1f);
  DataBlock test_zero(0.0f);

  for (uint32_t index = 0; index < test_result.samples.size(); index++) {
    test_result.samples[index] += value++;
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
    std::cout << "** is track in playback? " << result << " **" << std::endl;
    std::cout << "** m_c_i " << tm.GetMasterCurrentIndex() << std::endl;
    std::cout << "** t_c_i " << tm.tracks.at(track_number).GetCurrentIndex() << std::endl;
    // Verify data is 0 for blocks 0,1,6,7, rest should be test_result
    if (block_count >= data_block_number && block_count <= data_block_number_end) {
      result = AreBlocksMatching(tm.mixdown, test_result);
    } else {
      result = AreBlocksMatching(tm.mixdown, test_zero);
    }
    std::cout << " ** block " << block_count << " match? " << result << std::endl;
    tm.HandleIndexUpdate_AlreadyInState_AllStates();
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

  for (uint32_t index = 0; index < test_result.samples.size(); index++) {
    test_result.samples[index] += value++;
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
    std::cout << "** is track in playbackRepeat? " << result << " **" << std::endl;
    std::cout << "** m_c_i " << tm.GetMasterCurrentIndex() << std::endl;
    std::cout << "** t_c_i " << tm.tracks.at(track_number).GetCurrentIndex() << std::endl;
    // Verify data is test_result for all blocks
    result = AreBlocksMatching(tm.mixdown, test_result);
    std::cout << " ** block " << block_count << " match? " << result << std::endl;
    tm.HandleIndexUpdate_AlreadyInState_AllStates();
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

  for (uint32_t index = 0; index < test_result.samples.size(); index++) {
    test_result.samples[index] += value++;
  }


  // modify range to ensure mixdown only follows blocks within start and end for the track
  tm.tracks.at(track_number).SetStartIndex(data_block_number);
  tm.tracks.at(track_number).SetEndIndex(data_block_number_end);
  tm.SetMasterCurrentIndex(0);

  // Set track 0 to playback - mixdown relies on master_current
  tm.HandleStateChange_Playback(track_number);
  tm.HandleIndexUpdate_AlreadyInState_AllStates();

  std::cout << " *** entering loop for blocks 1+ " << std::endl;

  for (uint32_t block_count = 1; block_count < NUM_BLOCKS_OVERDUB + 1; block_count++) { 
    tm.HandleStateChange_Playback(track_number_test);

    // Verify state changed to recording
    result = tm.tracks.at(track_number).IsTrackInPlayback();
    std::cout << "** is track " << track_number << " in playback? " << result << " **" << std::endl;
    result = tm.tracks.at(track_number_test).IsTrackInPlayback();
    std::cout << "** is track " << track_number_test << " in playback? " << result << " **" << std::endl;

    std::cout << "** m_c_i " << tm.GetMasterCurrentIndex() << std::endl;
    std::cout << "** t_" << track_number << "_c_i " << tm.tracks.at(track_number).GetCurrentIndex() << std::endl;
    std::cout << "** t_" << track_number_test << "_c_i " << tm.tracks.at(track_number_test).GetCurrentIndex() << std::endl;

    // Verify data is 0 for blocks 0,1,6,7, rest should be test_result
    if (block_count >= data_block_number && block_count <= data_block_number_end) {
      result = AreBlocksMatching(tm.mixdown, test_result);
    } else {
      result = AreBlocksMatching(tm.mixdown, test_zero);
    }
    std::cout << " ** block " << block_count << " match? " << result << std::endl;
    if (!result) { tm.mixdown.PrintBlock(); }
    tm.HandleIndexUpdate_AlreadyInState_AllStates();
  }
}

void Test_MuteUnmute(TrackManager &tm) {
  std::cout << std::endl << std::endl << "** test_track_manager.cpp: Test_MuteUnmute **" << std::endl;
  uint16_t idx = 0;
  // Set all tracks to Playback
  for (auto &t : tm.tracks) {
    t.SetTrackToInPlayback();
  }

  // Mute some, leave others unmuted
  uint16_t mute_unmute = 0xAA55;
  bool result[16] = {true, false, true, false, true, false, true, false,
                     false, true, false, true, false, true, false, true};
  tm.HandleMuteUnmuteTracks(mute_unmute);
  bool test_passed = true;
  for (auto &t : tm.tracks) {
    if (result[idx] != t.IsTrackMuted()) {
      std::cout << " muted mismatch at " << idx << std::endl;
      std::cout << " exp " << result[idx] << ", act " << t.IsTrackMuted() << std::endl;
      test_passed = false;
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
  tm.HandleIndexUpdate_AlreadyInState_AllStates();

  gettimeofday(&t2, NULL);
  std::cout << t2.tv_sec - t1.tv_sec << "s, " << t2.tv_usec - t1.tv_usec << "us" << std::endl;

  // Verify state changed to recording
  result = tm.tracks.at(track_number).IsTrackInRecord();
  std::cout << "** is track in record? " << result << " **" << std::endl;

  // -> verify manually data and state - get data via mixdown - mixdown is currentIndex
  // mixdown should have been automatically performed
  result = AreBlocksMatching(tm.mixdown, test_data_1p1);
  std::cout << "** test_data_1p1 matched? " << result << " **" << std::endl << std::endl << std::endl;

  // -> Second test - insert first block of data on track 1 use test_data_1p1
  gettimeofday(&t1, NULL);

  tm.HandleStateChange_Recording(track_number_1, test_data_1p1);
  tm.HandleIndexUpdate_AlreadyInState_AllStates();

  gettimeofday(&t2, NULL);
  std::cout << t2.tv_sec - t1.tv_sec << "s, " << t2.tv_usec - t1.tv_usec << "us" << std::endl;

  // Verify state changed to recording
  result = tm.tracks.at(track_number_1).IsTrackInRecord();
  std::cout << "** is track " << track_number_1 << " in record? " << result << " **" << std::endl;

  // Verify first track state changed to Playback
  result = tm.tracks.at(track_number).IsTrackInPlayback();
  std::cout << "** is track " << track_number << " in payback? " << result << " **" << std::endl;

  // Verify indexes
  std::cout << "     track " << track_number << " start index " << tm.tracks.at(track_number).GetStartIndex() << std::endl;
  std::cout << "     track " << track_number << " current index " << tm.tracks.at(track_number).GetCurrentIndex() << std::endl;
  std::cout << "     track " << track_number << " end index " << tm.tracks.at(track_number).GetEndIndex() << std::endl;

  // Verify indexes
  std::cout << "     track " << track_number_1 << " start index " << tm.tracks.at(track_number_1).GetStartIndex() << std::endl;
  std::cout << "     track " << track_number_1 << " current index " << tm.tracks.at(track_number_1).GetCurrentIndex() << std::endl;
  std::cout << "     track " << track_number_1 << " end index " << tm.tracks.at(track_number_1).GetEndIndex() << std::endl;

  std::cout << "     master current index " << tm.GetMasterCurrentIndex() << ", end " << tm.GetMasterEndIndex() << std::endl;

  /// Repeat while switching tracks
  track_number = 1;
  track_number_1 = 0;

  gettimeofday(&t1, NULL);

  tm.HandleStateChange_Recording(track_number, test_data_1p1);
  tm.HandleIndexUpdate_AlreadyInState_AllStates();

  gettimeofday(&t2, NULL);
  std::cout << t2.tv_sec - t1.tv_sec << "s, " << t2.tv_usec - t1.tv_usec << "us" << std::endl;

  // Verify state changed to recording
  result = tm.tracks.at(track_number).IsTrackInRecord();
  std::cout << "** is track " << track_number << " in record? " << result << " **" << std::endl;

  // -> verify manually data and state - get data via mixdown - mixdown is currentIndex
  // mixdown should have been automatically performed
  result = AreBlocksMatching(tm.mixdown, test_data_1p1);
  std::cout << "** test_data_1p1 matched? " << result << " **" << std::endl << std::endl << std::endl;

  // -> Second test - insert first block of data on track 1 use test_data_1p1
  gettimeofday(&t1, NULL);

  tm.HandleStateChange_Recording(track_number_1, test_data_1p1);
  tm.HandleIndexUpdate_AlreadyInState_AllStates();

  gettimeofday(&t2, NULL);
  std::cout << t2.tv_sec - t1.tv_sec << "s, " << t2.tv_usec - t1.tv_usec << "us" << std::endl;

  // Verify state changed to recording
  result = tm.tracks.at(track_number_1).IsTrackInRecord();
  std::cout << "** is track " << track_number_1 << " in record? " << result << " **" << std::endl;

  // Verify first track state changed to Playback
  result = tm.tracks.at(track_number).IsTrackInPlayback();
  std::cout << "** is track " << track_number << " in payback? " << result << " **" << std::endl;

  // Verify indexes
  std::cout << "     track " << track_number << " start index " << tm.tracks.at(track_number).GetStartIndex() << std::endl;
  std::cout << "     track " << track_number << " current index " << tm.tracks.at(track_number).GetCurrentIndex() << std::endl;
  std::cout << "     track " << track_number << " end index " << tm.tracks.at(track_number).GetEndIndex() << std::endl;

  // Verify indexes
  std::cout << "     track " << track_number_1 << " start index " << tm.tracks.at(track_number_1).GetStartIndex() << std::endl;
  std::cout << "     track " << track_number_1 << " current index " << tm.tracks.at(track_number_1).GetCurrentIndex() << std::endl;
  std::cout << "     track " << track_number_1 << " end index " << tm.tracks.at(track_number_1).GetEndIndex() << std::endl;

  std::cout << "     master current index " << tm.GetMasterCurrentIndex() << ", end " << tm.GetMasterEndIndex() << std::endl;


}

int main() {
  std::cout << "** test_track_manager.cpp **" << std::endl;
  TrackManager tm;
  Test_Record_SingleTrack(tm);
  Test_Overdub_SingleTrack(tm);
  Test_Playback_SingleTrack(tm);
  Test_PlaybackRepeat_SingleTrack(tm);
  Test_Playback_DualTrack_SecondTrackNoData(tm);
  Test_MuteUnmute(tm);
  Test_RecordOnDiffTrack_NoPlaybackFirst(tm);


  return 0;
}
