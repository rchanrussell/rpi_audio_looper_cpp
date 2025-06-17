#ifndef TRACK_MANAGER_H
#define TRACK_MANAGER_H

#include <array>
#include <iostream>
#include <iterator>

#include "util.h"
#include "track.h"
#include "mixer.h"
#include "track_manager_state.h"
#include "output_i2c.h"

class OutputI2C;
class TrackManagerState;

class TrackManager {
#ifndef DTEST_TM
  std::array<Track, MAX_TRACK_COUNT> tracks;
#endif
  // Keep track of last track
  // if in Rec or Overdub and Rec/Overdub comes for another track, we must set
  // last track to play then proceed to perform operations on the 
  uint32_t last_track_number_;

  // no need for master start index, as it will always be zero! at least
  // one track must start at zero (if no audio desired, don't play, record silence)
  uint32_t master_end_index_;
  uint32_t master_current_index_;
  bool master_current_index_updated_;

  // DataBuffers
  // Input for Rec and Overdub
  DataBlock input_buffer_;

  // Output for all states
//  DataBlock mixdown;

#ifndef DTEST_TM
  // Active State Index Updates by State
  void IndexUpdateRecordNoChange(uint32_t track_number);
  void IndexUpdateOverdubNoChange(uint32_t track_number);
  void IndexUpdatePlaybackNoChange(uint32_t track_number);
  void IndexUpdateRepeatNoChange(uint32_t track_number);
  void IndexUpdateReachedEndOfAvailableSpace(uint32_t track_number);
#endif

  // Mixdown subfunctions
  uint32_t DetermineIndex(uint32_t track);
  void SilentPlaybackTrack(uint32_t track, uint32_t index);

  // State Machine Section
  TrackManagerState* current_state;
  OutputI2C* output_i2c;

  public:
  // Member variables
#ifdef DTEST_TM
  // For Tests
  std::array<Track, MAX_TRACK_COUNT> tracks;
  // Active State Index Updates by State
  void IndexUpdateRecordNoChange(uint32_t track_number);
  void IndexUpdateOverdubNoChange(uint32_t track_number);
  void IndexUpdatePlaybackNoChange(uint32_t track_number);
  void IndexUpdateRepeatNoChange(uint32_t track_number);
  void IndexUpdateReachedEndOfAvailableSpace(uint32_t track_number);

  void HandleStateChange_Recording(uint32_t track_number, DataBlock &data);
  void HandleStateChange_Overdubbing(uint32_t track_number, DataBlock &data);
  void HandleStateChange_Playback(uint32_t track_number);
  void HandleStateChange_PlaybackRepeat(uint32_t track_number);
  void HandleStateChange_Mute(uint32_t track_number);
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
  void IndexUpdateRecordEnter(uint32_t track_number);
  void IndexUpdateOverdubEnter(uint32_t track_number);
  void IndexUpdatePlaybackEnter(uint32_t track_number);
  void IndexUpdateRepeatEnter(uint32_t track_number);

  // On Exit
  void IndexUpdateRecordExit(uint32_t track_number); 
  void IndexUpdateOverdubExit(uint32_t track_number);
  void IndexUpdatePlaybackExit(uint32_t track_number);
  void IndexUpdateRepeatExit(uint32_t track_number);

  // Set Track State
  void SetTrackStateOff(uint32_t track_number);
  void SetTrackStateRecord(uint32_t track_number);
  void SetTrackStateOverdub(uint32_t track_number);
  void SetTrackStatePlayback(uint32_t track_number);
  void SetTrackStateRepeat(uint32_t track_number);
  void SetTrackStateMute(uint32_t track_number);

  // Reset condition -- if all tracks are off - reset our master current and end indexes
  bool AreAllTracksOff();

  // Active
  void IndexUpdateAllStatesNoChange();

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
  void SetOutputI2CPtr(OutputI2C* obj);
  uint16_t GetTracksInMute();
  uint16_t GetTracksInPlayback();
  uint16_t GetTracksOff();
};
#endif // TRACK_MANAGER_H
