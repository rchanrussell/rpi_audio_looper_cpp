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
}

void Off::exit(TrackManager &tm, uint32_t track_number) {
}

void Off::active(TrackManager &tm, uint32_t track_number) {
}

  // Event State Transitions
void Off::handle_down_event(TrackManager &tm, uint32_t track_number) {
  std::cout << "OFF:HDE->REC" << std::endl;
  // OFF -> RECORD
  tm.SetState(Record::getInstance(), track_number);
}

void Off::handle_double_down_event(TrackManager &tm, uint32_t track_number) {
  std::cout << "OFF:DDE" << std::endl;
}

void Off::handle_short_pulse_event(TrackManager &tm, uint32_t track_number) {
  std::cout << "OFF:SPE" << std::endl;
}

void Off::handle_long_pulse_event(TrackManager &tm, uint32_t track_number) {
  std::cout << "OFF:LPE" << std::endl;
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
  // Check if another track in Rec or Overdub - put it in Play
  // Then resume state transition for the new track
  // NOTE:if same track, this function just returns
  tm.NewTrackRecordingRequestWhileRecording(track_number);
  // Update indexes
  tm.HandleIndexUpdate_Recording_OnEnterState(track_number);
  // update track's state
  tm.SetTrackState_Record(track_number);

  // TODO make this only part of Active which is always called by
  // 'process' callback when data buffer is ready?
  // copy data to track
  // TODO implement buffer storage so this function doesn't
  // have to know where it came from, just supplies track_number and that's it
}

void Record::exit(TrackManager &tm, uint32_t track_number) {
  // Update Indexes
  tm.HandleIndexUpdate_Recording_OnExitState(track_number);
}

// Given TM is only in one state, that last one it transitioned too
// But tracks are in various states independent of one another, always
// call this in Active to ensure always updating when buffers come in/go out
void Record::active(TrackManager &tm, uint32_t track_number) {
  // Updeate Indexes
  tm.HandleIndexUpdate_AlreadyInState_AllStates();

  // copy data to track

  // perform mixdown
  tm.PerformMixdown();
  // whoever is calling TM (main app?) when the 'process' callback is called
  // it should handle copy the source buffer to the input buffer then
  // let this copy the data to the track
}

  // Event State Transitions
void Record::handle_down_event(TrackManager &tm, uint32_t track_number) {
  std::cout << "RECORD:HDE->PLY" << std::endl;
  // RECORD -> PLAY
  tm.SetState(Play::getInstance(), track_number);
}

void Record::handle_double_down_event(TrackManager &tm, uint32_t track_number) {
  std::cout << "RECORD:DDE" << std::endl;
}

void Record::handle_short_pulse_event(TrackManager &tm, uint32_t track_number) {
  std::cout << "RECORD:SPE" << std::endl;
}

void Record::handle_long_pulse_event(TrackManager &tm, uint32_t track_number) {
  std::cout << "RECORD:LPE->RPT" << std::endl;
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
  // Check if another track in Rec or Overdub - put it in Play
  // Then resume state transition for the new track
  // NOTE:if same track, this function just returns
  tm.NewTrackRecordingRequestWhileRecording(track_number);
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
  tm.HandleIndexUpdate_AlreadyInState_AllStates();

  // Copy track - mix with source - store to track
  tm.PerformMixdown();
}

  // Event State Transitions
void Overdub::handle_down_event(TrackManager &tm, uint32_t track_number) {
  std::cout << "OVERDUB:HDE->PLY" << std::endl;
  // OVERDUB -> PLAY
  tm.SetState(Play::getInstance(), track_number);
}

void Overdub::handle_double_down_event(TrackManager &tm, uint32_t track_number) {
  std::cout << "OVERDUB:DDE->MUT" << std::endl;
  tm.SetState(Mute::getInstance(), track_number);
}

void Overdub::handle_short_pulse_event(TrackManager &tm, uint32_t track_number) {
  std::cout << "OVERDUB:SPE" << std::endl;
}

void Overdub::handle_long_pulse_event(TrackManager &tm, uint32_t track_number) {
  std::cout << "OVERDUB:LPE->RPT" << std::endl;
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
  tm.HandleIndexUpdate_AlreadyInState_AllStates();
  tm.PerformMixdown();
}

  // Event State Transitions
void Play::handle_down_event(TrackManager &tm, uint32_t track_number) {
  std::cout << "PLAY:HDE->OVD" << std::endl;
  // PLAY -> OVERDUB
  tm.SetState(Overdub::getInstance(), track_number);
}

void Play::handle_double_down_event(TrackManager &tm, uint32_t track_number) {
  std::cout << "PLAY:DDE->OFF" << std::endl;
  tm.SetState(Off::getInstance(), track_number);
}

void Play::handle_short_pulse_event(TrackManager &tm, uint32_t track_number) {
  std::cout << "PLAY:SPE" << std::endl;
}

void Play::handle_long_pulse_event(TrackManager &tm, uint32_t track_number) {
  std::cout << "PLAY:LPE" << std::endl;
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
  tm.HandleIndexUpdate_AlreadyInState_AllStates();
  tm.PerformMixdown();
}

  // Event State Transitions
void Repeat::handle_down_event(TrackManager &tm, uint32_t track_number) {
  std::cout << "REPEAT:HDE->MUT" << std::endl;
  // REPEAT -> MUTE
  tm.SetState(Mute::getInstance(), track_number);
}

void Repeat::handle_double_down_event(TrackManager &tm, uint32_t track_number) {
  std::cout << "REPEAT:DDE" << std::endl;
}

void Repeat::handle_short_pulse_event(TrackManager &tm, uint32_t track_number) {
  std::cout << "REPEAT:SPE" << std::endl;
}

void Repeat::handle_long_pulse_event(TrackManager &tm, uint32_t track_number) {
  std::cout << "REPEAT:LPE" << std::endl;
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
  tm.HandleIndexUpdate_AlreadyInState_AllStates();
  tm.PerformMixdown();
}

  // Event State Transitions
void Mute::handle_down_event(TrackManager &tm, uint32_t track_number) {
  std::cout << "MUTE:HDE->PLY" << std::endl;
  // MUTE -> PLAY
  tm.SetState(Play::getInstance(), track_number);
}

void Mute::handle_double_down_event(TrackManager &tm, uint32_t track_number) {
  std::cout << "MUTE:DDE->OFF" << std::endl;
  tm.SetState(Off::getInstance(), track_number);
}

void Mute::handle_short_pulse_event(TrackManager &tm, uint32_t track_number) {
  std::cout << "MUTE:SPE" << std::endl;
}

void Mute::handle_long_pulse_event(TrackManager &tm, uint32_t track_number) {
  std::cout << "MUTE:LPE->RPT" << std::endl;
  tm.SetState(Repeat::getInstance(), track_number);
}

Mute& Mute::getInstance() {
  static Mute singleton;
  return singleton;
}

