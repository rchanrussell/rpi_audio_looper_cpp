#include "track.h"

void Track::SetTrackMembersToDefault() {
  start_index_ = 0;
  end_index_ = 0;
  current_index_ = 0;
  current_state_ = TrackState::kOff;
  previous_state_ = TrackState::kOff; // only used when muting/unmuting
  is_track_silent_ = true;
}

void Track::RestoreUsingSetState() {
  switch (previous_state_) {
    case TrackState::kOff:
      SetTrackToOff();
      break;
    case TrackState::kOverdub:
      SetTrackToOverdubbing();
      break;
    case TrackState::kPlayback:
      SetTrackToInPlayback();
      break;
    case TrackState::kRepeat:
      SetTrackToInPlaybackRepeat();
      break;
    case TrackState::kRecord:
      SetTrackToInRecord();
      break;
    case TrackState::kMuted:
      SetTrackToMuted();
      break;
  }
}

// Default Constructor - set all data to zero
Track::Track():Track(0.0f) {
}

Track::Track(float init_val) {
  DataBlock temp(init_val);
  for (auto& fb : frame_blocks) {
    fb.samples_ = temp.samples_;
  }
  SetTrackMembersToDefault();
}

void Track::SetBlockDataToSameValue(uint32_t block_number, float value) {
  frame_blocks.at(block_number).samples_.fill(value);
}

// Called by Record, TrackManager will send master current index
// to write the data to the correct block
void Track::SetBlockData(uint32_t block_number, DataBlock &block) {
  frame_blocks.at(block_number).samples_ = block.samples_;
}

const DataBlock & Track::GetBlockData(uint32_t block_number) {
  return frame_blocks.at(block_number);
}

void Track::SetStartIndex(uint32_t start) {
  start_index_ = start;
}

void Track::SetEndIndex(uint32_t end) {
  end_index_ = end;
}

void Track::SetCurrentIndex(uint32_t current) {
  current_index_ = current;
}

void Track::IncrementCurrentIndex() {
  current_index_++;
}

uint32_t Track::GetStartIndex() {
  return start_index_;
}

uint32_t Track::GetEndIndex() {
  return end_index_;
}

uint32_t Track::GetCurrentIndex() {
  return current_index_;
}

// Muted or Off -- no data transferred to mixer
// Include Overdub/Record in the mixdown so mixer should be called
// after transfer of data to the track when recording/overdubbing
bool Track::IsTrackSilent() {
  return is_track_silent_;
}
// If set to state MUTE, OFF or in playback but master_current_index is outside range
// of track's start and end indexes
void Track::SetTrackSilent(bool set_silent) {
  is_track_silent_ = set_silent;
}

bool Track::IsTrackOff() {
  return current_state_ == TrackState::kOff;
}

bool Track::IsTrackOverdubbing() {
  return current_state_ == TrackState::kOverdub;
}

bool Track::IsTrackInPlayback() {
  return current_state_ == TrackState::kPlayback;
}

bool Track::IsTrackInPlaybackRepeat() {
  return current_state_ == TrackState::kRepeat;
}

bool Track::IsTrackInRecord() {
  return current_state_ == TrackState::kRecord;
}

bool Track::IsTrackMuted() {
  return current_state_ == TrackState::kMuted;
}

void Track::SetTrackToOff() {
  SetTrackMembersToDefault();
}

void Track::SetTrackToOverdubbing() {
  SetTrackSilent(false);
  current_state_ = TrackState::kOverdub;
  previous_state_ = current_state_;
}

void Track::SetTrackToInPlayback() {
  SetTrackSilent(false);
  current_state_ = TrackState::kPlayback;
  previous_state_ = current_state_;
}

void Track::SetTrackToInPlaybackRepeat() {
  // always play data because Current is used not master_current like play
  SetTrackSilent(false);
  current_state_ = TrackState::kRepeat;
  previous_state_ = current_state_;
}

void Track::SetTrackToInRecord() {
  SetTrackSilent(false);
  current_state_ = TrackState::kRecord;
  previous_state_ = current_state_;
}

void Track::SaveCurrentState() {
  previous_state_ = current_state_;
}

void Track::RestoreCurrentState() {
  // Use this instead of current = previous as it will restore the is_silent flag aswell
  RestoreUsingSetState();
}

void Track::SetTrackToMuted() {
  SetTrackSilent(true);
  current_state_ = TrackState::kMuted;
}
