#include "track.h"

void Track::SetTrackMembersToDefault() {
  start_index = 0;
  end_index = 0;
  current_index = 0;
  current_state = TRACK_STATE_OFF;
  previous_state = TRACK_STATE_OFF; // only used when muting/unmuting
  is_track_silent = true;
}
void Track::RestoreUsingSetState() {
  switch (previous_state) {
    case TRACK_STATE_OFF:
      SetTrackToOff();
      break;
    case TRACK_STATE_OVERDUB:
      SetTrackToOverdubbing();
      break;
    case TRACK_STATE_PLAYBACK:
      SetTrackToInPlayback();
      break;
    case TRACK_STATE_PLAYBACK_REPEAT:
      SetTrackToInPlaybackRepeat();
      break;
    case TRACK_STATE_RECORDING:
      SetTrackToInRecord();
      break;
    case TRACK_STATE_MUTE:
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
    fb.samples = temp.samples;
  }
  SetTrackMembersToDefault();
}

void Track::SetBlockDataToSameValue(uint32_t block_number, float value) {
  frame_blocks.at(block_number).samples.fill(value);
}

// Called by Record, TrackManager will send master current index
// to write the data to the correct block
void Track::SetBlockData(uint32_t block_number, DataBlock &block) {
  frame_blocks.at(block_number).samples = block.samples;
}

const DataBlock & Track::GetBlockData(uint32_t block_number) {
  return frame_blocks.at(block_number);
}

void Track::SetStartIndex(uint32_t start) {
  start_index = start;
}

void Track::SetEndIndex(uint32_t end) {
  end_index = end;
}

void Track::SetCurrentIndex(uint32_t current) {
  current_index = current;
}

void Track::IncrementCurrentIndex() {
  current_index++;
}

uint32_t Track::GetStartIndex() {
  return start_index;
}

uint32_t Track::GetEndIndex() {
  return end_index;
}

uint32_t Track::GetCurrentIndex() {
  return current_index;
}

// Muted or Off -- no data transferred to mixer
// Include Overdub/Record in the mixdown so mixer should be called
// after transfer of data to the track when recording/overdubbing
bool Track::IsTrackSilent() {
  return is_track_silent;
}
// If set to state MUTE, OFF or in playback but master_current_index is outside range
// of track's start and end indexes
void Track::SetTrackSilent(bool set_silent) {
  is_track_silent = set_silent;
}

bool Track::IsTrackOff() {
  return current_state == TRACK_STATE_OFF;
}

bool Track::IsTrackOverdubbing() {
  return current_state == TRACK_STATE_OVERDUB;
}

bool Track::IsTrackInPlayback() {
  return current_state == TRACK_STATE_PLAYBACK;
}

bool Track::IsTrackInPlaybackRepeat() {
  return current_state == TRACK_STATE_PLAYBACK_REPEAT;
}

bool Track::IsTrackInRecord() {
  return current_state == TRACK_STATE_RECORDING;
}

bool Track::IsTrackMuted() {
  return current_state == TRACK_STATE_MUTE;
}

void Track::SetTrackToOff() {
  SetTrackMembersToDefault();
}

void Track::SetTrackToOverdubbing() {
  SetTrackSilent(false);
  current_state = TRACK_STATE_OVERDUB;
  previous_state = current_state;
}

void Track::SetTrackToInPlayback() {
  SetTrackSilent(false);
  current_state = TRACK_STATE_PLAYBACK;
  previous_state = current_state;
}

void Track::SetTrackToInPlaybackRepeat() {
  // always play data because Current is used not master_current like play
  SetTrackSilent(false);
  current_state = TRACK_STATE_PLAYBACK_REPEAT;
  previous_state = current_state;
}

void Track::SetTrackToInRecord() {
  SetTrackSilent(false);
  current_state = TRACK_STATE_RECORDING;
  previous_state = current_state;
}

void Track::SaveCurrentState() {
  previous_state = current_state;
}

void Track::RestoreCurrentState() {
  // Use this instead of current = previous as it will restore the is_silent flag aswell
  RestoreUsingSetState();
}

void Track::SetTrackToMuted() {
  SetTrackSilent(true);
  current_state = TRACK_STATE_MUTE;
}
