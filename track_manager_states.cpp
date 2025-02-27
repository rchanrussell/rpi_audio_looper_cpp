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
void Off::enter(TrackManager &tm, uint32_t track_number) {
  tm.SetTrackState_Off(track_number);
  tm.AreAllTracksOff(); // if true, it will automatically set indexes
}

void Off::exit(TrackManager &tm, uint32_t track_number) {
}

void Off::active(TrackManager &tm, uint32_t track_number) {
}

  // Event State Transitions
void Off::handle_down_event(TrackManager &tm, uint32_t track_number) {
  std::cout << "T:" << track_number << ":OFF:HDE->REC" << std::endl;
  // OFF -> RECORD
  tm.SetState(Record::getInstance(), track_number);
}

void Off::handle_double_down_event(TrackManager &tm, uint32_t track_number) {
  std::cout << "T:" << track_number << ":OFF:DDE" << std::endl;
}

void Off::handle_short_pulse_event(TrackManager &tm, uint32_t track_number) {
  std::cout << "T:" << track_number << ":OFF:SPE" << std::endl;
}

void Off::handle_long_pulse_event(TrackManager &tm, uint32_t track_number) {
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
void Record::enter(TrackManager &tm, uint32_t track_number) {
  // Update indexes
  tm.HandleIndexUpdate_Recording_OnEnterState(track_number);
  // update track's state
  tm.SetTrackState_Record(track_number);
}

void Record::exit(TrackManager &tm, uint32_t track_number) {
  // Update Indexes
  tm.HandleIndexUpdate_Recording_OnExitState(track_number);
}

// Given TM is only in one state, that last one it transitioned too
// But tracks are in various states independent of one another, always
// call this in Active to ensure always updating when buffers come in/go out
void Record::active(TrackManager &tm, uint32_t track_number) {
  // copy data to track
  tm.CopyBufferToTrack(track_number);

  // perform mixdown
  tm.PerformMixdown();

  // Updeate Indexes
  tm.HandleIndexUpdate_AlreadyInState_AllStates();
}

  // Event State Transitions
void Record::handle_down_event(TrackManager &tm, uint32_t track_number) {
  std::cout << "T:" << track_number << ":RECORD:HDE->PLY" << std::endl;
  // RECORD -> PLAY
  tm.SetState(Play::getInstance(), track_number);
}

void Record::handle_double_down_event(TrackManager &tm, uint32_t track_number) {
  std::cout << "T:" << track_number <<  "RECORD:DDE" << std::endl;
}

void Record::handle_short_pulse_event(TrackManager &tm, uint32_t track_number) {
  std::cout << "T:" << track_number << "RECORD:SPE" << std::endl;
}

void Record::handle_long_pulse_event(TrackManager &tm, uint32_t track_number) {
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
void Overdub::enter(TrackManager &tm, uint32_t track_number) {
  // Update indexes
  tm.HandleIndexUpdate_Overdubbing_OnEnterState(track_number);
  // update track's state
  tm.SetTrackState_Overdub(track_number);
}

void Overdub::exit(TrackManager &tm, uint32_t track_number) {
  tm.HandleIndexUpdate_Overdubbing_OnExitState(track_number);
}

// Given TM is only in one state, that last one it transitioned too
// But tracks are in various states independent of one another, always
// call this in Active to ensure always updating when buffers come in/go out
void Overdub::active(TrackManager &tm, uint32_t track_number) {
  tm.CopyBufferToTrack(track_number);
  tm.PerformMixdown();
  tm.HandleIndexUpdate_AlreadyInState_AllStates();
}

  // Event State Transitions
void Overdub::handle_down_event(TrackManager &tm, uint32_t track_number) {
  std::cout << "T:" << track_number << "OVERDUB:HDE->PLY" << std::endl;
  // OVERDUB -> PLAY
  tm.SetState(Play::getInstance(), track_number);
}

void Overdub::handle_double_down_event(TrackManager &tm, uint32_t track_number) {
  std::cout << "T:" << track_number << "OVERDUB:DDE->MUT" << std::endl;
  tm.SetState(Mute::getInstance(), track_number);
}

void Overdub::handle_short_pulse_event(TrackManager &tm, uint32_t track_number) {
  std::cout << "T:" << track_number << "OVERDUB:SPE" << std::endl;
}

void Overdub::handle_long_pulse_event(TrackManager &tm, uint32_t track_number) {
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
void Play::enter(TrackManager &tm, uint32_t track_number) {
  tm.HandleIndexUpdate_Playback_OnEnterState(track_number);
  tm.SetTrackState_Playback(track_number);
}

void Play::exit(TrackManager &tm, uint32_t track_number) {
  tm.HandleIndexUpdate_Playback_OnExitState(track_number);
}

void Play::active(TrackManager &tm, uint32_t track_number) {
  tm.PerformMixdown();
  tm.HandleIndexUpdate_AlreadyInState_AllStates();
}

  // Event State Transitions
void Play::handle_down_event(TrackManager &tm, uint32_t track_number) {
  std::cout << "T:" << track_number << "PLAY:HDE->OVD" << std::endl;
  // PLAY -> OVERDUB
  tm.SetState(Overdub::getInstance(), track_number);
}

void Play::handle_double_down_event(TrackManager &tm, uint32_t track_number) {
  std::cout << "T:" << track_number << "PLAY:DDE->OFF" << std::endl;
  tm.SetState(Off::getInstance(), track_number);
}

void Play::handle_short_pulse_event(TrackManager &tm, uint32_t track_number) {
  std::cout << "T:" << track_number << "PLAY:SPE" << std::endl;
}

void Play::handle_long_pulse_event(TrackManager &tm, uint32_t track_number) {
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
void Repeat::enter(TrackManager &tm, uint32_t track_number) {
  tm.HandleIndexUpdate_PlaybackRepeat_OnEnterState(track_number);
  tm.SetTrackState_Repeat(track_number);
}

void Repeat::exit(TrackManager &tm, uint32_t track_number) {
  tm.HandleIndexUpdate_PlaybackRepeat_OnExitState(track_number);
}

void Repeat::active(TrackManager &tm, uint32_t track_number) {
  tm.PerformMixdown();
  tm.HandleIndexUpdate_AlreadyInState_AllStates();
}

  // Event State Transitions
void Repeat::handle_down_event(TrackManager &tm, uint32_t track_number) {
  std::cout << "T:" << track_number << "REPEAT:HDE->MUT" << std::endl;
  // REPEAT -> MUTE
  tm.SetState(Mute::getInstance(), track_number);
}

void Repeat::handle_double_down_event(TrackManager &tm, uint32_t track_number) {
  std::cout << "T:" << track_number << "REPEAT:DDE" << std::endl;
}

void Repeat::handle_short_pulse_event(TrackManager &tm, uint32_t track_number) {
  std::cout << "T:" << track_number << "REPEAT:SPE" << std::endl;
}

void Repeat::handle_long_pulse_event(TrackManager &tm, uint32_t track_number) {
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
void Mute::enter(TrackManager &tm, uint32_t track_number) {
  tm.SetTrackState_Mute(track_number);
}

void Mute::exit(TrackManager &tm, uint32_t track_number) {
}

void Mute::active(TrackManager &tm, uint32_t track_number) {
  tm.PerformMixdown();
  tm.HandleIndexUpdate_AlreadyInState_AllStates();
}

  // Event State Transitions
void Mute::handle_down_event(TrackManager &tm, uint32_t track_number) {
  std::cout << "T:" << track_number << "MUTE:HDE->PLY" << std::endl;
  // MUTE -> PLAY
  tm.SetState(Play::getInstance(), track_number);
}

void Mute::handle_double_down_event(TrackManager &tm, uint32_t track_number) {
  std::cout << "T:" << track_number << "MUTE:DDE->OFF" << std::endl;
  tm.SetState(Off::getInstance(), track_number);
}

void Mute::handle_short_pulse_event(TrackManager &tm, uint32_t track_number) {
  std::cout << "T:" << track_number << "MUTE:SPE" << std::endl;
}

void Mute::handle_long_pulse_event(TrackManager &tm, uint32_t track_number) {
  std::cout << "T:" << track_number << "MUTE:LPE->RPT" << std::endl;
  tm.SetState(Repeat::getInstance(), track_number);
}

Mute& Mute::getInstance() {
  static Mute singleton;
  return singleton;
}

