#include "group_manager_states.h"

/*
 * NOT_ACTIVE
 */
// State Specific Methods
void NotActive::enter(GroupManager &gm, TrackManager &tm, uint32_t group_number, uint32_t track_number) {
}

void NotActive::exit(GroupManager &gm, TrackManager &tm, uint32_t group_number, uint32_t track_number) {
}

void NotActive::active(GroupManager &gm, TrackManager &tm, uint32_t group_number, uint32_t track_number) {
}

  // Event State Transitions
void NotActive::handle_down_event(GroupManager &gm, TrackManager &tm, uint32_t group_number, uint32_t track_number) {
  std::cout << "G:" << group_number << ":NOT_ACTIVE:HDE->ACTIVE" << std::endl;
  gm.SetState(Active::getInstance(), tm, group_number, track_number);
}

void NotActive::handle_double_down_event(GroupManager &gm, TrackManager &tm, uint32_t group_number, uint32_t track_number) {
  std::cout << "G:" << group_number << ":NOT_ACTIVE:DDE" << std::endl;
}

void NotActive::handle_short_pulse_event(GroupManager &gm, TrackManager &tm, uint32_t group_number, uint32_t track_number) {
  std::cout << "G:" << group_number << ":NOT_ACTIVE:SPE" << std::endl;
}

void NotActive::handle_long_pulse_event(GroupManager &gm, TrackManager &tm, uint32_t group_number, uint32_t track_number) {
  std::cout << "G:" << group_number << "NOT_ACTIVE:LPE" << std::endl;
}

NotActive& NotActive::getInstance() {
  static NotActive singleton;
  return singleton;
}

/*
 * ACTIVE
 */
// State Specific Methods
void Active::enter(GroupManager &gm, TrackManager &tm, uint32_t group_number, uint32_t track_number) {
  gm.SetActiveGroup(group_number, tm);
}

void Active::exit(GroupManager &gm, TrackManager &tm, uint32_t group_number, uint32_t track_number) {
}

void Active::active(GroupManager &gm, TrackManager &tm, uint32_t group_number, uint32_t track_number) {
}

  // Event State Transitions
void Active::handle_down_event(GroupManager &gm, TrackManager &tm, uint32_t group_number, uint32_t track_number) {
  std::cout << "G:" << group_number << ":ACTIVE:HDE->ADD_TRACK" << std::endl;
  // ACTIVE -> ADD_TRACK
  gm.SetState(AddTrack::getInstance(), tm, group_number, track_number);
}

void Active::handle_double_down_event(GroupManager &gm, TrackManager &tm, uint32_t group_number, uint32_t track_number) {
  std::cout << "G:" << group_number <<  "ACTIVE:DDE->ADD_TRACK" << std::endl;
  gm.SetState(AddTrack::getInstance(), tm, group_number, track_number);
}

void Active::handle_short_pulse_event(GroupManager &gm, TrackManager &tm, uint32_t group_number, uint32_t track_number) {
  std::cout << "G:" << group_number << "ACTIVE:SPE" << std::endl;
}

void Active::handle_long_pulse_event(GroupManager &gm, TrackManager &tm, uint32_t group_number, uint32_t track_number) {
  std::cout << "G:" << group_number << "ACTIVE:LPE" << std::endl;
}

Active& Active::getInstance() {
  static Active singleton;
  return singleton;
}

/*
 * ADD_TRACK
 */
// Last event received was a overdub event - copy data from source
void AddTrack::enter(GroupManager &gm, TrackManager &tm, uint32_t group_number, uint32_t track_number) {
}

void AddTrack::exit(GroupManager &gm, TrackManager &tm, uint32_t group_number, uint32_t track_number) {
}

void AddTrack::active(GroupManager &gm, TrackManager &tm, uint32_t group_number, uint32_t track_number) {
  gm.AddTrackToGroup(track_number, group_number);
}

  // Event State Transitions
void AddTrack::handle_down_event(GroupManager &gm, TrackManager &tm, uint32_t group_number, uint32_t track_number) {
  std::cout << "G:" << group_number << "ADD_TRACK:HDE->ACTIVE" << std::endl;
  // ADD_TRACK->ACTIVE
  gm.SetState(Active::getInstance(), tm, group_number, track_number);
}

void AddTrack::handle_double_down_event(GroupManager &gm, TrackManager &tm, uint32_t group_number, uint32_t track_number) {
  std::cout << "G:" << group_number << "ADD_TRACK:DDE->NOT_ACTIVE" << std::endl;
  gm.SetState(NotActive::getInstance(), tm, group_number, track_number);
}

void AddTrack::handle_short_pulse_event(GroupManager &gm, TrackManager &tm, uint32_t group_number, uint32_t track_number) {
  std::cout << "G:" << group_number << "ADD_TRACK:SPE" << std::endl;
}

void AddTrack::handle_long_pulse_event(GroupManager &gm, TrackManager &tm, uint32_t group_number, uint32_t track_number) {
  std::cout << "G:" << group_number << "ADD_TRACK:LPE->REMOVE_ALL_TRACKS" << std::endl;
  // ADD_TRACK -> REMOVE_ALL_TRACKS
  gm.SetState(RemoveAllTracks::getInstance(), tm, group_number, track_number);
}

AddTrack& AddTrack::getInstance() {
  static AddTrack singleton;
  return singleton;
}

/*
 * REMOVE_ALL_TRACKS
 */
// Last event received was a play event
void RemoveAllTracks::enter(GroupManager &gm, TrackManager &tm, uint32_t group_number, uint32_t track_number) {
}

void RemoveAllTracks::exit(GroupManager &gm, TrackManager &tm, uint32_t group_number, uint32_t track_number) {
}

void RemoveAllTracks::active(GroupManager &gm, TrackManager &tm, uint32_t group_number, uint32_t track_number) {
}

  // Event State Transitions
void RemoveAllTracks::handle_down_event(GroupManager &gm, TrackManager &tm, uint32_t group_number, uint32_t track_number) {
  std::cout << "G:" << group_number << "REMOVE_ALL_TRACKS:HDE->NOT_ACTIVE" << std::endl;
  // REMOVE_ALL_TRACKS -> NOT_ACTIVE
  gm.SetState(NotActive::getInstance(), tm, group_number, track_number);
}

void RemoveAllTracks::handle_double_down_event(GroupManager &gm, TrackManager &tm, uint32_t group_number, uint32_t track_number) {
  std::cout << "G:" << group_number << "REMOVE_ALL_TRACKS:DDE" << std::endl;
}

void RemoveAllTracks::handle_short_pulse_event(GroupManager &gm, TrackManager &tm, uint32_t group_number, uint32_t track_number) {
  std::cout << "G:" << group_number << "REMOVE_ALL_TRACKS:SPE" << std::endl;
}

void RemoveAllTracks::handle_long_pulse_event(GroupManager &gm, TrackManager &tm, uint32_t group_number, uint32_t track_number) {
  std::cout << "G:" << group_number << "REMOVE_ALL_TRACKS:LPE" << std::endl;
}

RemoveAllTracks& RemoveAllTracks::getInstance() {
  static RemoveAllTracks singleton;
  return singleton;
}

