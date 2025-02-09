#ifndef TRACK_MANAGER_H
#define TRACK_MANAGER_H

#include <array>
#include <iostream>
#include <iterator>
#include "util.h"
#include "track.h"
#include "mixer.h"


enum SystemEvents
{
  SYSTEM_EVENT_PASSTHROUGH,               // System->passthrough, all tracks->off, all indexes set to 0
  SYSTEM_EVENT_RECORD_TRACK,              // System->recording, track->recording - track & group # required
  SYSTEM_EVENT_OVERDUB_TRACK,             // System->overdubbing, track->recording - track & group # required
  SYSTEM_EVENT_PLAY_TRACK,                // Reset's track's current index to start index and state to play - track # required
  SYSTEM_EVENT_MUTE_TRACK,                // Place particular track into Mute state - track # required
  SYSTEM_EVENT_UNMUTE_TRACK,              // Changes track to Play state - track # required
  SYSTEM_EVENT_ADD_TRACK_TO_GROUP,        // Adds a track to a group - nothing more - track # & group # required
  SYSTEM_EVENT_REMOVE_TRACK_FROM_GROUP,   // Removes track from a group - track # & group # required
  SYSTEM_EVENT_SET_ACTIVE_GROUP,          // Sets the currently active group - group # required
};

enum SystemStates
{
  SYSTEM_STATE_PASSTHROUGH,       // No mixdown or recording
  SYSTEM_STATE_PLAYBACK,          // Tracks available for mixing and playing
  SYSTEM_STATE_RECORDING,         // Copying data to selected track
  SYSTEM_STATE_OVERDUBBING,       // Overdubbing selected track
  SYSTEM_STATE_CALIBRATION        // For sychronization configuration
};


// TODO State Machine of events
// Sets track state based upon event received
// Transfer data from input audio buffer to track in recording
// Perform mixdown on tracks not silent and transfer data to output audio buffer
// 

class TrackManager {
#ifndef DTEST_TM
  std::array<Track, MAX_TRACK_COUNT> tracks;
#endif
  // no need for master start index, as it will always be zero! at least
  // one track must start at zero (if no audio desired, don't play, record silence)
  uint32_t master_end_index;
  uint32_t master_current_index;
  bool master_current_index_updated;
  void HandleIndexUpdate_Recording_OnEnterState(uint32_t track_number);
  void HandleIndexUpdate_Recording_AlreadyInState(uint32_t track_number);
  void HandleIndexUpdate_Recording_OnExitState(uint32_t track_number); 
  void HandleIndexUpdate_Overdubbing_OnEnterState(uint32_t track_number);
  void HandleIndexUpdate_Overdubbing_AlreadyInState(uint32_t track_number);
  void HandleIndexUpdate_Overdubbing_OnExitState(uint32_t track_number);
  void HandleIndexUpdate_Playback_OnEnterState(uint32_t track_number);
  void HandleIndexUpdate_Playback_AlreadyInState(uint32_t track_number);
  void HandleIndexUpdate_Playback_OnExitState(uint32_t track_number);
  void HandleIndexUpdate_PlaybackRepeat_OnEnterState(uint32_t track_number);
  void HandleIndexUpdate_PlaybackRepeat_AlreadyInState(uint32_t track_number);
  void HandleIndexUpdate_PlaybackRepeat_OnExitState(uint32_t track_number);
  void HandleIndexUpdate_ReachedEndOfAvailableSpace(uint32_t track_number);

  public:
  // Member variables
#ifdef DTEST_TM
  std::array<Track, MAX_TRACK_COUNT> tracks;
#endif
  DataBlock mixdown;
  // Member Functions
  TrackManager();

  void PerformMixdown();
  void SetMasterCurrentIndex(uint32_t current);
  void SetMasterEndIndex(uint32_t end);
  uint32_t GetMasterCurrentIndex();
  uint32_t GetMasterEndIndex();

  void HandleStateChange_Recording(uint32_t track_number, DataBlock &data);
  void HandleStateChange_Overdubbing(uint32_t track_number, DataBlock &data);
  void HandleStateChange_Playback(uint32_t track_number);
  void HandleStateChange_PlaybackRepeat(uint32_t track_number);
  void HandleIndexUpdate_AlreadyInState_AllStates();
  void HandleMuteUnmuteTracks(uint16_t tracks);

/*
  These should also be called by GroupManager - Group Manager keeps track of which Tracks are in
  which group and uses the Mute/Unmute to set the tracks to play or not
  Using a uint16_t so we can go by bits - if 1 - track unmuted, if 0 - track muted
  Muted state tracks are also silent so not included in mixdown
  When entering Playback it will update indexes to master_current_index, so muting should not
  do that
  Should unmuting set to play? Do we need unmute? If track was empty, why mute and if unmute
  it does nothing because of the start_/end_index values are not set
  As we saw with testing, unused tracks are silent even in play mode
  void HandleMuteTracks(uint16_t tracks);
  void HandleUnmuteTracks(uint16_t tracks);
*/
};
#endif // TRACK_MANAGER_H
