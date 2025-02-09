#ifndef TRACK_H
#define TRACK_H

#include <array>
#include <iostream>
#include <iterator>
#include "data_block.h"

enum TrackState {
  TRACK_STATE_OFF,                // Empty track or available for recording
  TRACK_STATE_OVERDUB,            // In overdub mode, may/may not update start/end indicies
  TRACK_STATE_PLAYBACK,           // In playback mode
  TRACK_STATE_PLAYBACK_REPEAT,    // In playback mode with repeat
  TRACK_STATE_RECORDING,          // In recording mode - overwrites any previous recording info
  TRACK_STATE_MUTE                // In mute state, don't mixdown

};

// TODO Update to number based on model of RPI
// 512b/128 samples in 2.9ms or 512b/0.003s
// or 170667b/s
// if 6GB used that's 2359s per track!
// set to a reasonable number based upon RPI model and weather stereo or not
// For stereo we'd create another track manager object

class Track {
  // Indexes are per block of 128 samples, not per sample
  uint32_t start_index;
  uint32_t end_index;
  uint32_t current_index;
  bool is_track_silent;
  TrackState current_state;
  TrackState previous_state;
  void SetTrackMembersToDefault();
  void RestoreUsingSetState();

  public:
  // Member Variables - while private provides encapsulation, I don't want copying
  // so where possible, access the data directly when reading
  std::array<DataBlock, MAX_BLOCK_COUNT> frame_blocks;
  // Member Functions
  Track();
  Track(float init_val);
  void SetBlockDataToSameValue(uint32_t block_number, float value);
  void SetBlockData(uint32_t block_number, DataBlock &block);
  const DataBlock & GetBlockData(uint32_t block_number);

  void SetStartIndex(uint32_t start);
  void SetEndIndex(uint32_t end);
  void SetCurrentIndex(uint32_t current);
  // Used in repeat, rather than have caller read to local var then incr then store
  void IncrementCurrentIndex();
  uint32_t GetStartIndex();
  uint32_t GetEndIndex();
  uint32_t GetCurrentIndex();

  // Handles Off or Muted or Playback when master_current_index outside track's start_index
  // and end_index range
  bool IsTrackSilent();
  void SetTrackSilent(bool set_silent);

  bool IsTrackOff();
  bool IsTrackOverdubbing();
  bool IsTrackInPlayback();
  bool IsTrackInPlaybackRepeat();
  bool IsTrackInRecord();
  bool IsTrackMuted();

  void SetTrackToOff();
  void SetTrackToOverdubbing();
  void SetTrackToInPlayback();
  void SetTrackToInPlaybackRepeat();
  void SetTrackToInRecord();
  void SaveCurrentState();
  void RestoreCurrentState();
  void SetTrackToMuted();
};
#endif // TRACK_H
