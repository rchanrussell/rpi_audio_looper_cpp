#ifndef TRACK_MANAGER_H
#define TRACK_MANAGER_H

#include <array>
#include <iostream>
#include <iterator>
#include "util.h"
#include "track.h"
#include "mixer.h"
#include "track_manager_state.h"


enum SystemEvents
{
/* IDLE is from a separate input and requires a separate handler
* These below are actually track events
* If all tracks are off the system is in idle state or the separate input process
* will do this with a single procedure
*/
  SYSTEM_EVENT_IDLE,                      // System->passthrough, all tracks->off, all indexes set to 0
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
// These are actually track and system state hybrid - if all tracks off - system is idle
// but some tracks will be in various states OFF/REC/OVERDUB/PLAY/REPEAT/MUTE
// The system really just tracks the last track state and doesn't have it's own
// concept of a state other than off/operating
// copying of buffer depends on the last track state (or TM's current state)
// You can't have a track in record and the current state of TM not be record
// Only time TM is in record or overdub is if active track is there
// Playback or more corectly, mixdown data transfer is enabled always
// Tracks that are off/playback/repeat/mute will transfer data (or blocks of zeroes)
// depending on the indexes and state (mute/off always send zeroes)
// Rec/Overdub also always send data as we copy the data, mixit and send it out with the rest
// from the other tracks
  SYSTEM_STATE_IDLE,              // No mixdown or recording
  SYSTEM_STATE_PLAYBACK,          // Tracks available for mixing and playing
  SYSTEM_STATE_PLAYBACK_REPEAT,   // Tracks available for mixing and playing
  SYSTEM_STATE_RECORDING,         // Copying data to selected track
  SYSTEM_STATE_OVERDUBBING,       // Overdubbing selected track
};


// TODO State Machine of events
// Sets track state based upon event received
// Transfer data from input audio buffer to track in recording
// Perform mixdown on tracks not silent and transfer data to output audio buffer
// 

class TrackManagerState;

class TrackManager {
#ifndef DTEST_TM
  std::array<Track, MAX_TRACK_COUNT> tracks;
#endif
  // Keep track of last track
  // if in Rec or Overdub and Rec/Overdub comes for another track, we must set
  // last track to play then proceed to perform operations on the 
  uint32_t last_track_number;

  // no need for master start index, as it will always be zero! at least
  // one track must start at zero (if no audio desired, don't play, record silence)
  uint32_t master_end_index;
  uint32_t master_current_index;
  bool master_current_index_updated;

  // DataBuffers
  // Input for Rec and Overdub
  DataBlock input_buffer;

  // Output for all states
//  DataBlock mixdown;

  // Active State Index Updates by State
  void HandleIndexUpdate_Recording_AlreadyInState(uint32_t track_number);
  void HandleIndexUpdate_Overdubbing_AlreadyInState(uint32_t track_number);
  void HandleIndexUpdate_Playback_AlreadyInState(uint32_t track_number);
  void HandleIndexUpdate_PlaybackRepeat_AlreadyInState(uint32_t track_number);
  void HandleIndexUpdate_ReachedEndOfAvailableSpace(uint32_t track_number);

  // State Machine Section
  TrackManagerState* current_state;

  public:
  // Member variables
#ifdef DTEST_TM
  std::array<Track, MAX_TRACK_COUNT> tracks;
#endif

  // TODO make private, add getter function
  DataBlock mixdown;

  // Member Functions
  TrackManager();

  void PerformMixdown();

  // For both group manager and testing
  // Group Manager deals with changing (it stores it) MasterEndIndex
  // Whenever a group change happens
  void SetMasterCurrentIndex(uint32_t current);
  void SetMasterEndIndex(uint32_t end);
  uint32_t GetMasterCurrentIndex();
  uint32_t GetMasterEndIndex();

  // For Tests
  void HandleStateChange_Recording(uint32_t track_number, DataBlock &data);
  void HandleStateChange_Overdubbing(uint32_t track_number, DataBlock &data);
  void HandleStateChange_Playback(uint32_t track_number);
  void HandleStateChange_PlaybackRepeat(uint32_t track_number);
  void HandleStateChange_Mute(uint32_t track_number);

  // This is for Group Manager which knows which tracks it needs to mute/unmute
  // so no getter required, just a simple single function call to simplify code
  void HandleMuteUnmuteTracks(uint16_t tracks);

  // State Machine Section
  void SetState(TrackManagerState &new_state, uint32_t track_number);
  inline TrackManagerState& GetCurrentState() const { return *current_state; }
  void SyncTrackManagerStateWithTrackState(uint32_t track_number);
  // transfer data, perform mixdown, update indexes
  void StateProcess(uint32_t track_number);

  /*
   * TODO Organize better the state related stuff from before
   * OnEnter by state
   * OnExit by state
   * NEW: OnActive by state (Update all indexes, this will copy data to track if REC/OVD
   * Separately perform Mixdown and Copy Mixdown Out
   * --- OnActive must be called after OnEnter -- always -- also updateAllStatesIndexes
   */

  // Index Updaters By State -- State will call appropriate one on Enter and Exit
  // Note that Mute and Off do not require any index updates - they don't transfer data
  // anyway, so when we leave those states the indexes will be updated to where they
  // should be
  // On Enter
  void HandleIndexUpdate_Recording_OnEnterState(uint32_t track_number);
  void HandleIndexUpdate_Overdubbing_OnEnterState(uint32_t track_number);
  void HandleIndexUpdate_Playback_OnEnterState(uint32_t track_number);
  void HandleIndexUpdate_PlaybackRepeat_OnEnterState(uint32_t track_number);

  // On Exit
  void HandleIndexUpdate_Recording_OnExitState(uint32_t track_number); 
  void HandleIndexUpdate_Overdubbing_OnExitState(uint32_t track_number);
  void HandleIndexUpdate_Playback_OnExitState(uint32_t track_number);
  void HandleIndexUpdate_PlaybackRepeat_OnExitState(uint32_t track_number);

  // Set Track State
  void SetTrackState_Off(uint32_t track_number);
  void SetTrackState_Record(uint32_t track_number);
  void SetTrackState_Overdub(uint32_t track_number);
  void SetTrackState_Playback(uint32_t track_number);
  void SetTrackState_Repeat(uint32_t track_number);
  void SetTrackState_Mute(uint32_t track_number);

  // Reset condition -- if all tracks are off - reset our master current and end indexes
  bool AreAllTracksOff();

  // Active
  void HandleIndexUpdate_AlreadyInState_AllStates();

  // Data Transfers Copy To Track
  void CopyBufferToTrack(uint32_t track_number);

  // Use void* and size in bytes? then copy that to a DataBlock
  void CopyToInputBuffer(void *data, uint32_t nsamples);

  // Data Transfers always -- Copy Mixdown
  // Accept pointer to write mixdown DataBlock data to buffer
  void CopyMixdownToBuffer(void *data, uint32_t nsamples);

  // Call the current state's versions
  void HandleDownEvent(uint32_t track_number);
  void HandleDoubleDownEvent(uint32_t track_number);
  void HandleShortPulseEvent(uint32_t track_number);
  void HandleLongPulseEvent(uint32_t track_number);
  

};
#endif // TRACK_MANAGER_H
