#include "track_manager_states.h"

/*
 * Always, any state, the mixdown is performed and data is
 * copied to the output buffer
 */

/*
 * OFF
 */
// All tracks are off - system is in idle
// State Specific Methods
void Off::Enter(TrackManager &tm, uint32_t track_number) {
std::cout << "Off:Enter t:" << track_number << std::endl;
  tm.SetTrackStateOff(track_number);
  tm.UpdateMasterEndIndex();
  tm.AreAllTracksOff(); // if true, it will automatically reset master indexes
}

void Off::Exit(TrackManager &tm, uint32_t track_number) {
}

void Off::Active(TrackManager &tm, uint32_t track_number) {
// last state entered was off, if user wants to off -> rec on same we need to remain off
// HOWEVER if there's at least one track in playback or repeat, we should continue mixdown
  if (tm.GetTracksInMute() > 0 || tm.GetTracksInPlayback() > 0) {
    tm.PerformMixdown();
    tm.IndexUpdateAllStatesNoChange();
  }
}

  // Event State Transitions
void Off::DownEvent(TrackManager &tm, uint32_t track_number) {
  std::cout << "T:" << track_number << ":OFF:HDE->REC" << std::endl;
  // OFF -> RECORD
  tm.SetState(Record::getInstance(), track_number);
}

void Off::DoubleDownEvent(TrackManager &tm, uint32_t track_number) {
  std::cout << "T:" << track_number << ":OFF:DDE" << std::endl;
}

void Off::ShortPulseEvent(TrackManager &tm, uint32_t track_number) {
  std::cout << "T:" << track_number << ":OFF:SPE" << std::endl;
}

void Off::LongPulseEvent(TrackManager &tm, uint32_t track_number) {
  std::cout << "T:" << track_number << "OFF:LPE" << std::endl;
}

Off& Off::getInstance() {
  static Off singleton;
  return singleton;
}

/*
 * RECORD
 */
// Last event received was a record event - copy data from source
// State Specific Methods
void Record::Enter(TrackManager &tm, uint32_t track_number) {
  // Update indexes
  tm.IndexUpdateRecordEnter(track_number);
  // update track's state
  tm.SetTrackStateRecord(track_number);
}

void Record::Exit(TrackManager &tm, uint32_t track_number) {
  // Update Indexes
  tm.IndexUpdateRecordExit(track_number);
}

// Given TM is only in one state, that last one it transitioned too
// But tracks are in various states independent of one another, always
// call this in Active to ensure always updating when buffers come in/go out
void Record::Active(TrackManager &tm, uint32_t track_number) {
  // copy data to track
  tm.CopyBufferToTrack(track_number);

  // perform mixdown
  tm.PerformMixdown();

  // Updeate Indexes
  tm.IndexUpdateAllStatesNoChange();
}

  // Event State Transitions
void Record::DownEvent(TrackManager &tm, uint32_t track_number) {
  std::cout << "T:" << track_number << ":RECORD:HDE->PLY" << std::endl;
  // RECORD -> PLAY
  tm.SetState(Play::getInstance(), track_number);
}

void Record::DoubleDownEvent(TrackManager &tm, uint32_t track_number) {
  std::cout << "T:" << track_number <<  "RECORD:DDE" << std::endl;
}

void Record::ShortPulseEvent(TrackManager &tm, uint32_t track_number) {
  std::cout << "T:" << track_number << "RECORD:SPE" << std::endl;
}

void Record::LongPulseEvent(TrackManager &tm, uint32_t track_number) {
  std::cout << "T:" << track_number << "RECORD:LPE->RPT" << std::endl;
  // RECORD -> REPEAT
  tm.SetState(Repeat::getInstance(), track_number);
}

Record& Record::getInstance() {
  static Record singleton;
  return singleton;
}

/*
 * OVERDUB
 */
// Last event received was a overdub event - copy data from source
void Overdub::Enter(TrackManager &tm, uint32_t track_number) {
  // Update indexes
  tm.IndexUpdateOverdubEnter(track_number);
  // update track's state
  tm.SetTrackStateOverdub(track_number);
}

void Overdub::Exit(TrackManager &tm, uint32_t track_number) {
  tm.IndexUpdateOverdubExit(track_number);
}

// Given TM is only in one state, that last one it transitioned too
// But tracks are in various states independent of one another, always
// call this in Active to ensure always updating when buffers come in/go out
void Overdub::Active(TrackManager &tm, uint32_t track_number) {
  tm.CopyBufferToTrack(track_number);
  tm.PerformMixdown();
  tm.IndexUpdateAllStatesNoChange();
}

  // Event State Transitions
void Overdub::DownEvent(TrackManager &tm, uint32_t track_number) {
  std::cout << "T:" << track_number << "OVERDUB:HDE->PLY" << std::endl;
  // OVERDUB -> PLAY
  tm.SetState(Play::getInstance(), track_number);
}

void Overdub::DoubleDownEvent(TrackManager &tm, uint32_t track_number) {
  std::cout << "T:" << track_number << "OVERDUB:DDE->MUT" << std::endl;
  tm.SetState(Mute::getInstance(), track_number);
}

void Overdub::ShortPulseEvent(TrackManager &tm, uint32_t track_number) {
  std::cout << "T:" << track_number << "OVERDUB:SPE" << std::endl;
}

void Overdub::LongPulseEvent(TrackManager &tm, uint32_t track_number) {
  std::cout << "T:" << track_number << "OVERDUB:LPE->RPT" << std::endl;
  // OVERDUB -> REPEAT
  tm.SetState(Repeat::getInstance(), track_number);
}

Overdub& Overdub::getInstance() {
  static Overdub singleton;
  return singleton;
}

/*
 * PLAY
 */
// Last event received was a play event
void Play::Enter(TrackManager &tm, uint32_t track_number) {
  tm.IndexUpdatePlaybackEnter(track_number);
  tm.SetTrackStatePlayback(track_number);
}

void Play::Exit(TrackManager &tm, uint32_t track_number) {
  tm.IndexUpdatePlaybackExit(track_number);
}

void Play::Active(TrackManager &tm, uint32_t track_number) {
  tm.PerformMixdown();
  tm.IndexUpdateAllStatesNoChange();
}

  // Event State Transitions
void Play::DownEvent(TrackManager &tm, uint32_t track_number) {
  std::cout << "T:" << track_number << "PLAY:HDE->OVD" << std::endl;
  // PLAY -> OVERDUB
  tm.SetState(Overdub::getInstance(), track_number);
}

void Play::DoubleDownEvent(TrackManager &tm, uint32_t track_number) {
  std::cout << "T:" << track_number << "PLAY:DDE->OFF" << std::endl;
  tm.SetState(Off::getInstance(), track_number);
}

void Play::ShortPulseEvent(TrackManager &tm, uint32_t track_number) {
  std::cout << "T:" << track_number << "PLAY:SPE" << std::endl;
}

void Play::LongPulseEvent(TrackManager &tm, uint32_t track_number) {
  std::cout << "T:" << track_number << "PLAY:LPE" << std::endl;
}

Play& Play::getInstance() {
  static Play singleton;
  return singleton;
}

/*
 * REPEAT
 */
// Last event received was a repeat event
void Repeat::Enter(TrackManager &tm, uint32_t track_number) {
  tm.IndexUpdateRepeatEnter(track_number);
  tm.SetTrackStateRepeat(track_number);
}

void Repeat::Exit(TrackManager &tm, uint32_t track_number) {
  tm.IndexUpdateRepeatExit(track_number);
}

void Repeat::Active(TrackManager &tm, uint32_t track_number) {
  tm.PerformMixdown();
  tm.IndexUpdateAllStatesNoChange();
}

  // Event State Transitions
void Repeat::DownEvent(TrackManager &tm, uint32_t track_number) {
  std::cout << "T:" << track_number << "REPEAT:HDE->MUT" << std::endl;
  // REPEAT -> MUTE
  tm.SetState(Mute::getInstance(), track_number);
}

void Repeat::DoubleDownEvent(TrackManager &tm, uint32_t track_number) {
  std::cout << "T:" << track_number << "REPEAT:DDE" << std::endl;
}

void Repeat::ShortPulseEvent(TrackManager &tm, uint32_t track_number) {
  std::cout << "T:" << track_number << "REPEAT:SPE" << std::endl;
}

void Repeat::LongPulseEvent(TrackManager &tm, uint32_t track_number) {
  std::cout << "T:" << track_number << "REPEAT:LPE" << std::endl;
}

Repeat& Repeat::getInstance() {
  static Repeat singleton;
  return singleton;
}

/*
 * MUTE
 */
// Last event received was a mute event
void Mute::Enter(TrackManager &tm, uint32_t track_number) {
  tm.SetTrackStateMute(track_number);
}

void Mute::Exit(TrackManager &tm, uint32_t track_number) {
}

void Mute::Active(TrackManager &tm, uint32_t track_number) {
  tm.PerformMixdown();
  tm.IndexUpdateAllStatesNoChange();
}

  // Event State Transitions
void Mute::DownEvent(TrackManager &tm, uint32_t track_number) {
  std::cout << "T:" << track_number << "MUTE:HDE->PLY" << std::endl;
  // MUTE -> PLAY
  tm.SetState(Play::getInstance(), track_number);
}

void Mute::DoubleDownEvent(TrackManager &tm, uint32_t track_number) {
  std::cout << "T:" << track_number << "MUTE:DDE->OFF" << std::endl;
  tm.SetState(Off::getInstance(), track_number);
}

void Mute::ShortPulseEvent(TrackManager &tm, uint32_t track_number) {
  std::cout << "T:" << track_number << "MUTE:SPE" << std::endl;
}

void Mute::LongPulseEvent(TrackManager &tm, uint32_t track_number) {
  std::cout << "T:" << track_number << "MUTE:LPE->RPT" << std::endl;
  tm.SetState(Repeat::getInstance(), track_number);
}

Mute& Mute::getInstance() {
  static Mute singleton;
  return singleton;
}

