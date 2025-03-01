#include "track_manager.h"
#include "track_manager_states.h"

static DataBlock empty_block;

// Default Constructor - set all data to zero
TrackManager::TrackManager() {
  current_state = &Off::getInstance();
}

// Handle Index
uint32_t TrackManager::DetermineIndex(uint32_t track) {
    return tracks.at(track).IsTrackInPlaybackRepeat() ?
           tracks.at(track).GetCurrentIndex() : master_current_index_;
}

void TrackManager::SilentPlaybackTrack(uint32_t track, uint32_t index) {
  uint32_t track_start_index = 0;
  uint32_t track_end_index = 0;

  if (tracks.at(track).IsTrackInPlayback()) {
    track_start_index = tracks.at(track).GetStartIndex();
    track_end_index = tracks.at(track).GetEndIndex();

    if (index < track_start_index || index > track_end_index || track_start_index == track_end_index) {
      tracks.at(track).SetTrackSilent(true);
    } else {
      tracks.at(track).SetTrackSilent(false);
    }
  }
}

// Perform Mixdown
// Pass empty_block in place of tracks off/muted/in other group
void TrackManager::PerformMixdown() {
  uint32_t index_one = 0;
  uint32_t index_two = 0;

  // Clear mixdown as MixBlocks does not do this and shouldn't for simplicity
  mixdown.SetData(empty_block);

  for (int t = 0; t < tracks.size(); t+=2) {
    // Handle index
    index_one = DetermineIndex(t);
    index_two = DetermineIndex(t + 1);

    // tracks in playback when master_current_index_ is outside range
    // need to be set to silent
    SilentPlaybackTrack(t, index_one);
    SilentPlaybackTrack(t + 1, index_two);

    // Handle block mixdown based upon state
    // TODO - non-active group needs to be in here too
    if (tracks.at(t).IsTrackSilent() && tracks.at(t + 1).IsTrackSilent()) {
      MixBlocks(empty_block,
                empty_block,
                mixdown);
    } else if (tracks.at(t).IsTrackSilent() && !tracks.at(t + 1).IsTrackSilent()) {
      MixBlocks(empty_block,
                tracks.at(t + 1).GetBlockData(index_two),
                mixdown);
    } else if (!tracks.at(t).IsTrackSilent() && tracks.at(t + 1).IsTrackSilent()) {
      MixBlocks(tracks.at(t).GetBlockData(index_one),
                empty_block,
                mixdown);
    } else {
      MixBlocks(tracks.at(t).GetBlockData(index_one),
                tracks.at(t + 1).GetBlockData(index_two),
                mixdown);
    }
  }
}

/*
 * Index Handlers
 */

void TrackManager::SetMasterCurrentIndex(uint32_t current) {
  master_current_index_ = current;
}
void TrackManager::SetMasterEndIndex(uint32_t end) {
  master_end_index_ = end;
}
uint32_t TrackManager::GetMasterCurrentIndex() {
  return master_current_index_;
}
uint32_t TrackManager::GetMasterEndIndex() {
  return master_end_index_;
}

// If master_current_index_ reaches max available space, reset it and change state if
// required
void TrackManager::IndexUpdateReachedEndOfAvailableSpace(uint32_t track_number) {
  // master always starts at 0
  master_current_index_ = 0;
  tracks.at(track_number).SetCurrentIndex(master_current_index_);
}

/*
 * Index Handlers On Entry of State
 */

// Handle Index Update - Recording
// OnEnterState (from another state)
// -> Set CurrentIndex to MasterCurrentIndex
// -> Set StartIndex to CurrentIndex
void TrackManager::IndexUpdateRecordEnter(uint32_t track_number) {
  tracks.at(track_number).SetCurrentIndex(master_current_index_);
  tracks.at(track_number).SetStartIndex(master_current_index_);
}

// Handle Index Update - Overdubbing
// OnEnterState (from another state)
// -> Set CurrentIndex to MasterCurrentIndex
// -> If CurrentIndex < StartIndex, Set StartIndex to CurrentIndex
void TrackManager::IndexUpdateOverdubEnter(uint32_t track_number) {
  tracks.at(track_number).SetCurrentIndex(master_current_index_);
  if (master_current_index_ < tracks.at(track_number).GetStartIndex()) {
    tracks.at(track_number).SetStartIndex(master_current_index_);
  }
}
 
// Handle Index Update - Playback
// OnEnterState (from another state)
// -> Set CurrentIndex to MasterCurrentIndex
void TrackManager::IndexUpdatePlaybackEnter(uint32_t track_number) {
  // If entering play and MCI == MEI, set MCI to 0, IE restart from beginning
  if (master_current_index_ >= master_end_index_) {
    master_current_index_ = 0;
  }
  tracks.at(track_number).SetCurrentIndex(master_current_index_);
}

// Handle Index Update - PlaybackRepeat
// OnEnterState (from another state)
// -> Set CurrentIndex to StartIndex
void TrackManager::IndexUpdateRepeatEnter(uint32_t track_number) {
  // If entering repeat and MCI == MEI, set MCI to 0, IE restart from beginning
  if (master_current_index_ >= master_end_index_) {
    master_current_index_ = 0;
  }
  tracks.at(track_number).SetCurrentIndex(tracks.at(track_number).GetStartIndex());
}

/*
 * Index Handlers Already In State
 */

// Re-enterState (from Recording state)
// -> Increment CurrentIndex
void TrackManager::IndexUpdateRecordNoChange(uint32_t track_number) {
  tracks.at(track_number).IncrementCurrentIndex();
  if (!master_current_index_updated_) {
    master_current_index_++;
    master_current_index_updated_ = true;
  }
  // If we're at the end of available data space for the master track:
  // Change from Recording to Playback, reset indexes
  if (master_current_index_ == MAX_BLOCK_COUNT) {
    IndexUpdateReachedEndOfAvailableSpace(track_number);
    tracks.at(track_number).SetTrackToInPlayback();
  }
}
 
// Re-enterState (from Overdubbing state)
// -> Increment CurrentIndex
void TrackManager::IndexUpdateOverdubNoChange(uint32_t track_number) {
  tracks.at(track_number).IncrementCurrentIndex();
  if (!master_current_index_updated_) {
    master_current_index_++;
    master_current_index_updated_ = true;
  }
  // If we're at the end of available data space for the master track:
  // Change from Overdubbing to Playback, reset indexes
  if (master_current_index_ == MAX_BLOCK_COUNT) {
    IndexUpdateReachedEndOfAvailableSpace(track_number);
    tracks.at(track_number).SetTrackToInPlayback();
  }
}

// Re-enterState (from Playback state)
// -> Increment CurrentIndex
void TrackManager::IndexUpdatePlaybackNoChange(uint32_t track_number) {
  tracks.at(track_number).IncrementCurrentIndex();
  if (!master_current_index_updated_) {
    master_current_index_++;
    master_current_index_updated_ = true;
  }
  // If we're at the end of available data space for the master track:
  if (master_current_index_ == MAX_BLOCK_COUNT) {
    IndexUpdateReachedEndOfAvailableSpace(track_number);
  }
  // ensure not recording - which will be current_state if we are
  // this track may be in playback but another could be rec/ovd which means
  // it will be increasing master end index
  if (tracks.at(last_track_number_).IsTrackOverdubbing() ||
      tracks.at(last_track_number_).IsTrackInRecord()) {
    return;
  }
  if (master_current_index_ > master_end_index_) {
    master_current_index_ = 0;
    tracks.at(track_number).SetCurrentIndex(master_current_index_);
  }
}

// Re-enterState (from PlaybackRepeat state)
// -> Increment CurrentIndex
// -> If CurrentIndex > EndIndex, set CurrentIndex to StartIndex
void TrackManager::IndexUpdateRepeatNoChange(uint32_t track_number) {
  tracks.at(track_number).IncrementCurrentIndex();
  if (!master_current_index_updated_) {
    master_current_index_++;
    master_current_index_updated_ = true;
  }

  if (tracks.at(track_number).GetCurrentIndex() > tracks.at(track_number).GetEndIndex()) {
    tracks.at(track_number).SetCurrentIndex(tracks.at(track_number).GetStartIndex());
  } 

  // If we're at the end of available data space for the master track:
  // Don't call generic handler as we don't want to update the track's current index
  // while in Repeat because we're not in sync with master_current_index_, we stay within
  // the track's own range
  if (master_current_index_ == MAX_BLOCK_COUNT) {
    // master always starts at 0
    master_current_index_ = 0;
  }
  // ensure not recording - which will be current_state if we are
  // this track may be in playback but another could be rec/ovd which means
  // it will be increasing master end index
  if (tracks.at(last_track_number_).IsTrackOverdubbing() ||
      tracks.at(last_track_number_).IsTrackInRecord()) {
    return;
  }
  if (master_current_index_ > master_end_index_) {
    master_current_index_ = 0;
    tracks.at(track_number).SetCurrentIndex(master_current_index_);
  }
}

/*
 * Index Handlers Exit State - when changing to another state
 */

// OnExitState (leaving to another state)
// -> Set EndIndex to CurrentIndex
// -> If CurrentIndex > MasterEndIndex, Set MasterEndIndex to CurrentIndex
void TrackManager::IndexUpdateRecordExit(uint32_t track_number) {
  uint32_t current_index = master_current_index_;
  tracks.at(track_number).SetEndIndex(current_index);
  if (current_index > master_end_index_) {
    master_end_index_ = current_index;
  }
}

// OnExitState (leaving to another state)
// -> If CurrentIndex > EndIndex, Set EndIndex to CurrentIndex
// -> If CurrentIndex > MasterEndIndex, Set MasterEndIndex to CurrentIndex
void TrackManager::IndexUpdateOverdubExit(uint32_t track_number) {
  uint32_t current_index = master_current_index_;
  if (current_index > tracks.at(track_number).GetEndIndex()) {
    tracks.at(track_number).SetEndIndex(current_index);
  }
  if (current_index > master_end_index_) {
    master_end_index_ = current_index;
  }
}

// OnExitState (leaving to another state)
// -> No changes required
void TrackManager::IndexUpdatePlaybackExit(uint32_t track_number) {
}

// OnExitState (leaving to another state)
// -> No changes required
void TrackManager::IndexUpdateRepeatExit(uint32_t track_number) {
}


// Non-state Change Index Update Handler
// Required because.. some tracks will be in playback while another is in focus in recording
// There is nothing to handle the current index of non-focus tracks under this condition
// Also, if all tracks are in playback, master needs to be updated
// Unless all tracks are in off, update master here
// A flag exists because we should update the master current index only once
void TrackManager::IndexUpdateAllStatesNoChange() {
  // loop through all tracks
  master_current_index_updated_ = false;
  for (uint32_t track_number = 0; track_number < tracks.size(); track_number++) {
    if (tracks.at(track_number).IsTrackOff()) {
      continue;
    }
    if (tracks.at(track_number).IsTrackInRecord()) {
      IndexUpdateRecordNoChange(track_number);
      // Update the _end_index 
      continue; // don't waste time doing rest of the checks
    }
    if (tracks.at(track_number).IsTrackOverdubbing()) {
      IndexUpdateOverdubNoChange(track_number);
      continue; // don't waste time doing rest of the checks
    }
    // Mute is playback but silent, so keep indexes updating
    if (tracks.at(track_number).IsTrackInPlayback() ||
        tracks.at(track_number).IsTrackMuted()) {
      IndexUpdatePlaybackNoChange(track_number);
      continue; // don't waste time doing rest of the checks
    }
    if (tracks.at(track_number).IsTrackInPlaybackRepeat()) {
      IndexUpdateRepeatNoChange(track_number);
      continue; // don't waste time doing rest of the checks
    }
  }
}

/*
 * Set Track State Handlers
 */
void TrackManager::SetTrackStateOff(uint32_t track_number) {
  tracks.at(track_number).SetTrackToOff();
}

void TrackManager::SetTrackStateRecord(uint32_t track_number) {
  tracks.at(track_number).SetTrackToInRecord();
}

void TrackManager::SetTrackStateOverdub(uint32_t track_number) {
  tracks.at(track_number).SetTrackToOverdubbing();
}

void TrackManager::SetTrackStatePlayback(uint32_t track_number) {
  tracks.at(track_number).SetTrackToInPlayback();
}

void TrackManager::SetTrackStateRepeat(uint32_t track_number) {
  tracks.at(track_number).SetTrackToInPlaybackRepeat();
}

void TrackManager::SetTrackStateMute(uint32_t track_number) {
  tracks.at(track_number).SetTrackToMuted();
}


/*
 * State Change Handlers Used by tests
 */
#ifdef DTEST_TM
// Handle State Change - Recording(track_number, data)
// -> Caller should provide pointer to buffer from audio source
void TrackManager::HandleStateChange_Recording(uint32_t track_number, DataBlock &data) {

  // TODO: This should be somewhere else?
  static uint32_t current_track = 0;
  if (current_track != track_number) {
    if (tracks.at(current_track).IsTrackInRecord()) {
      IndexUpdateRecordExit(track_number);
      tracks.at(current_track).SetTrackToInPlayback();
      current_track = track_number;
    }
  }

  if (!tracks.at(track_number).IsTrackInRecord()) {
    // OnEnterState
    // -> HandleIndexUpdate_Recording_OnEnter
    IndexUpdateRecordEnter(track_number);
  }
  // InRecording
  // -> SetBlockData(GetCurrentIndex(), DataBlock data)
  tracks.at(track_number).SetBlockData(tracks.at(track_number).GetCurrentIndex(),
                                       data);
  // -> SetStateToInRecord()
  tracks.at(track_number).SetTrackToInRecord();
  // -> PerformMixdown
  PerformMixdown();
  // -> Caller should copy mixdown which is why it is public

  // OnExitState
  // -> HandleIndexUpdate_Recording_OnExit
  // IndexUpdateRecordExit(track_number);
}

// Handle State Change - Overdubbing(track_number, data)
void TrackManager::HandleStateChange_Overdubbing(uint32_t track_number, DataBlock &data) {
  if (!tracks.at(track_number).IsTrackOverdubbing()) {
    // OnEnterState
    // -> HandleIndexUpdate_Overdubbing_OnEnter
    IndexUpdateOverdubEnter(track_number);
  }
  // InOverdubbing
  // -> Create temp block for mixing
  uint32_t current_index = tracks.at(track_number).GetCurrentIndex();
  DataBlock temp_block;
  // -> MixBlocks(track.GetBlockData(CurrentBlockIndex),
  //              data,
  //              temp_block)
  MixBlocks(tracks.at(track_number).GetBlockData(current_index), data, temp_block);
  // -> track.SetBlockData(CurrentBlockIndex, temp_block)
#ifdef DTEST_TM_VERBOSE
std::cout << "  *** SetBlockData Overdub curr_idx " << current_index << std::endl;
std::cout << "      master_current_index_ " << master_current_index_ << std::endl;
temp_block.PrintBlock();
#endif

  tracks.at(track_number).SetBlockData(current_index, temp_block);
  // -> SetStateToOverdubbing()
  tracks.at(track_number).SetTrackToOverdubbing();
  // -> PerformMixdown
  PerformMixdown();
  // -> Caller should copy mixdown

  // OnExitState
  // -> HandleIndexUpdate_Overdubbing_OnExit
  // IndexUpdateOverdubExit(track_number);
}

// Handle State Change - Playback(track_number)
void TrackManager::HandleStateChange_Playback(uint32_t track_number) {

  // Don't change indexes if was muted
  if (!tracks.at(track_number).IsTrackInPlayback() &&
      !tracks.at(track_number).IsTrackMuted()) {
    // OnEnterState
    // -> HandleIndexUpdate_Playback_OnEnter
    IndexUpdatePlaybackEnter(track_number);
  }
  // InPlayback
  // -> SetStateToInPlayback()
  tracks.at(track_number).SetTrackToInPlayback();
  // -> PerformMixdown
  PerformMixdown();
  // -> Caller should copy mixdown

  // OnExitState
  // -> HandleIndexUpdate_Playback_OnExit
  // IndexUpdatePlaybackExit(track_number);
}

// Handle State Change - PlaybackRepeat(track_number)
void TrackManager::HandleStateChange_PlaybackRepeat(uint32_t track_number) {
  if (!tracks.at(track_number).IsTrackInPlaybackRepeat()) {
    // OnEnterState
    // -> HandleIndexUpdate_PlaybackRepeat_OnEnter
    IndexUpdateRepeatEnter(track_number);
  }
  // InPlaybackRepeat
  // -> SetStateToInPlaybackRepeat()
  tracks.at(track_number).SetTrackToInPlaybackRepeat();
  // -> PerformMixdown
  PerformMixdown();
  // -> Caller should copy mixdown

  // OnExitState
  // -> HandleIndexUpdate_Playback_OnExit
  // IndexUpdateRepeatExit(track_number);
}

// Handle State Change - Mute (track_number)
void TrackManager::HandleStateChange_Mute(uint32_t track_number) {
  tracks.at(track_number).SetTrackToMuted();
  PerformMixdown();
}
#endif


// Mute tracks where 1 is set, 0 to unmute
void TrackManager::HandleMuteUnmuteTracks(uint16_t tracks_to_mute_unmute) {
  // loop through all tracks
  // save current state so we can return to it when unmuting
  for (uint32_t track_number = 0; track_number < tracks.size(); track_number++) {
    if (tracks_to_mute_unmute & (0x1 << track_number)) {
      // if already muted, don't mute again, as this will make restoring impossible
      // this will prevent consecutive muted groups from destroying record of last unmuted group
      if (!tracks.at(track_number).IsTrackMuted()) {
        tracks.at(track_number).SaveCurrentState();
      }
      tracks.at(track_number).SetTrackToMuted();
    } else {
      tracks.at(track_number).RestoreCurrentState();
    }
  }
}

/*
 * State Machine Methods
 */

void TrackManager::SyncTrackManagerStateWithTrackState(uint32_t track_number) {
  if (last_track_number_ == track_number) {
    return;
  }
  // Special case -- if previous operation was a record/overdub on a different track
  // we must set it to playback then continue with syncing
  if (tracks.at(last_track_number_).IsTrackOverdubbing() ||
      tracks.at(last_track_number_).IsTrackInRecord()) {
      SetState(Play::getInstance(), last_track_number_);
  }
  if (tracks.at(track_number).IsTrackOff()) {
    current_state = &Off::getInstance();
    return;
  }
  if (tracks.at(track_number).IsTrackOverdubbing()) {
    current_state = &Overdub::getInstance();
    return;
  }
  if (tracks.at(track_number).IsTrackInPlayback()) {
    current_state = &Play::getInstance();
    return;
  }
  if (tracks.at(track_number).IsTrackInPlaybackRepeat()) {
    current_state = &Repeat::getInstance();
    return;
  }
  if (tracks.at(track_number).IsTrackInRecord()) {
    current_state = &Record::getInstance();
    return;
  }
  if (tracks.at(track_number).IsTrackMuted()) {
    current_state = &Mute::getInstance();
    return;
  }

}

void TrackManager::SetState(TrackManagerState &new_state, uint32_t track_number) {
  current_state->Exit(*this, track_number);
  current_state = &new_state;
  current_state->Enter(*this, track_number);
}

/*
 * Data Transfer Methods
 */

void TrackManager::CopyBufferToTrack(uint32_t track_number) {
  tracks.at(track_number).SetBlockData(tracks.at(track_number).GetCurrentIndex(), input_buffer_);
}

void TrackManager::CopyToInputBuffer(void *d, uint32_t nsamples) {
  int *data = (int *)d;
  if (nsamples > SAMPLES_PER_BLOCK) {
    std::copy(data, data + SAMPLES_PER_BLOCK, begin(input_buffer_.samples_));
  } else {
    std::copy(data, data + nsamples, begin(input_buffer_.samples_));
  }
}

void TrackManager::CopyMixdownToBuffer(void *d, uint32_t nsamples) {
  int *data = (int *)d;
  if (nsamples > SAMPLES_PER_BLOCK) {
    std::copy(begin(mixdown.samples_), end(mixdown.samples_), data);
  } else {
    std::copy(begin(mixdown.samples_), begin(mixdown.samples_) + nsamples, data);
  }
}

/*
 * Event Handlers
 * event system knows the event type D, DD, SP, LP and which input/track
 * We want to store this track number and pass it on as the various index updates
 * and data copies require it -- so we know which track should state transition
 * TM always follows the current track's transition
 * But in some cases we should handle forced transitions
 * IE recording on T0, user presses DN on T1 - we must stop recording on T0 - set it to PLY
 */

// Design Change:
// When changing tracks, we must sync our current state with that of the track
// IE: T1 off, T0 REC, T1 DN EVENT -> T0 rec so send Down Event to it
// Sync state machine with T1 (ie: Off) pass Down Event to T1
// If not special case ie: REC/OVD, then still resync SM to new track

void TrackManager::HandleDownEvent(uint32_t track_number) {
  // sync with track
  SyncTrackManagerStateWithTrackState(track_number);
  current_state->DownEvent(*this, track_number);
  last_track_number_ = track_number;
}

void TrackManager::HandleDoubleDownEvent(uint32_t track_number) {
  // sync with track
  SyncTrackManagerStateWithTrackState(track_number);
  current_state->DoubleDownEvent(*this, track_number);
  last_track_number_ = track_number;
}

void TrackManager::HandleShortPulseEvent(uint32_t track_number) {
  // sync with track
  SyncTrackManagerStateWithTrackState(track_number);
  current_state->ShortPulseEvent(*this, track_number);
  last_track_number_ = track_number;
}

void TrackManager::HandleLongPulseEvent(uint32_t track_number) {
  // sync with track
  SyncTrackManagerStateWithTrackState(track_number);
  current_state->LongPulseEvent(*this, track_number);
  last_track_number_ = track_number;
}

void TrackManager::StateProcess(uint32_t track_number) {
  SyncTrackManagerStateWithTrackState(track_number);
  current_state->Active(*this, track_number);
}

bool TrackManager::AreAllTracksOff() {
  for (auto &t : tracks) {
    if (!t.IsTrackOff()) {
      return false;
    }
  }
  // reset master's indexes
  master_current_index_ = 0;
  master_end_index_ = 0;
  return true;
}

