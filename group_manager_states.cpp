#include "group_manager_states.h"

/*
 * NOT_ACTIVE
 */
// State Specific Methods
void NotActive::enter(GroupManager &gm, TrackManager &tm, uint32_t group_number, uint32_t track_number) {
  gm.SetActiveGroup(group_number, tm);
  // User changed active group to not active state - same group
  // Silence all tracks but don't reset any pointers
  if (gm.GetActiveGroup() == group_number) {
    gm.SilenceAllTracks(tm);
  }
  gm.GroupInactive(group_number);
}

void NotActive::exit(GroupManager &gm, TrackManager &tm, uint32_t group_number, uint32_t track_number) {
}

void NotActive::active(GroupManager &gm, TrackManager &tm, uint32_t group_number, uint32_t track_number) {
}

  // Event State Transitions
void NotActive::handle_down_event(GroupManager &gm, TrackManager &tm, uint32_t group_number, uint32_t track_number) {
#ifdef DTEST_VERBOSE_GM
  std::cout << "G:" << group_number << ":NOT_ACTIVE:HDE->ACTIVE" << std::endl;
#endif
  gm.SetState(Active::getInstance(), tm, group_number, track_number);
}

void NotActive::handle_double_down_event(GroupManager &gm, TrackManager &tm, uint32_t group_number, uint32_t track_number) {
#ifdef DTEST_VERBOSE_GM
  std::cout << "G:" << group_number << ":NOT_ACTIVE:DDE" << std::endl;
#endif
}

void NotActive::handle_short_pulse_event(GroupManager &gm, TrackManager &tm, uint32_t group_number, uint32_t track_number) {
#ifdef DTEST_VERBOSE_GM
  std::cout << "G:" << group_number << ":NOT_ACTIVE:SPE" << std::endl;
#endif
}

void NotActive::handle_long_pulse_event(GroupManager &gm, TrackManager &tm, uint32_t group_number, uint32_t track_number) {
#ifdef DTEST_VERBOSE_GM
  std::cout << "G:" << group_number << "NOT_ACTIVE:LPE" << std::endl;
#endif
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
  gm.GroupActive(group_number);
  // User changed not active group to active state - same group
  // Unmute group's tracks but don't reset any pointers
  if (gm.GetActiveGroup() == group_number) {
    gm.UnmuteActiveGroupTracks(tm);
  } else {
    gm.SetActiveGroup(group_number, tm);
  }
  // Should a track be turned off, we want to ensure the end indices are only within the
  // active group
  tm.SetActiveGroupTracks(gm.GetTracksInGroup(group_number));
}

void Active::exit(GroupManager &gm, TrackManager &tm, uint32_t group_number, uint32_t track_number) {
}

void Active::active(GroupManager &gm, TrackManager &tm, uint32_t group_number, uint32_t track_number) {
}

  // Event State Transitions
void Active::handle_down_event(GroupManager &gm, TrackManager &tm, uint32_t group_number, uint32_t track_number) {
  if (gm.GetActiveGroup() == group_number) {
#ifdef DTEST_VERBOSE_GM
    std::cout << "G:" << group_number << ":ACTIVE:HDE->ADD_TRACK" << std::endl;
#endif
    gm.SetState(AddTrack::getInstance(), tm, group_number, track_number);
  } else {
#ifdef DTEST_VERBOSE_GM
    std::cout << "G:" << group_number << ":ACTIVE:HDE->ACTIVE_NEW_GROUP" << std::endl;
#endif
    // If the group number is different, re-enter the Active state
    gm.SetState(Active::getInstance(), tm, group_number, track_number);
  }
}

void Active::handle_double_down_event(GroupManager &gm, TrackManager &tm, uint32_t group_number, uint32_t track_number) {
#ifdef DTEST_VERBOSE_GM
  std::cout << "G:" << group_number <<  "ACTIVE:DDE->ADD_TRACK" << std::endl;
#endif
  gm.SetState(AddTrack::getInstance(), tm, group_number, track_number);
}

void Active::handle_short_pulse_event(GroupManager &gm, TrackManager &tm, uint32_t group_number, uint32_t track_number) {
#ifdef DTEST_VERBOSE_GM
  std::cout << "G:" << group_number << "ACTIVE:SPE" << std::endl;
#endif
}

void Active::handle_long_pulse_event(GroupManager &gm, TrackManager &tm, uint32_t group_number, uint32_t track_number) {
#ifdef DTEST_VERBOSE_GM
  std::cout << "G:" << group_number << "ACTIVE:LPE" << std::endl;
#endif
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
  gm.GroupAddTrack(group_number);
  gm.SetActiveGroup(group_number, tm);
}

void AddTrack::exit(GroupManager &gm, TrackManager &tm, uint32_t group_number, uint32_t track_number) {
}

void AddTrack::active(GroupManager &gm, TrackManager &tm, uint32_t group_number, uint32_t track_number) {
  gm.AddTrackToGroup(track_number, group_number);
}

  // Event State Transitions
void AddTrack::handle_down_event(GroupManager &gm, TrackManager &tm, uint32_t group_number, uint32_t track_number) {
#ifdef DTEST_VERBOSE_GM
  std::cout << "G:" << group_number << "ADD_TRACK:HDE->ACTIVE" << std::endl;
#endif
  // ADD_TRACK->ACTIVE
  gm.SetState(Active::getInstance(), tm, group_number, track_number);
}

void AddTrack::handle_double_down_event(GroupManager &gm, TrackManager &tm, uint32_t group_number, uint32_t track_number) {
#ifdef DTEST_VERBOSE_GM
  std::cout << "G:" << group_number << "ADD_TRACK:DDE->NOT_ACTIVE" << std::endl;
#endif
  gm.SetState(NotActive::getInstance(), tm, group_number, track_number);
}

void AddTrack::handle_short_pulse_event(GroupManager &gm, TrackManager &tm, uint32_t group_number, uint32_t track_number) {
#ifdef DTEST_VERBOSE_GM
  std::cout << "G:" << group_number << "ADD_TRACK:SPE" << std::endl;
#endif
}

void AddTrack::handle_long_pulse_event(GroupManager &gm, TrackManager &tm, uint32_t group_number, uint32_t track_number) {
#ifdef DTEST_VERBOSE_GM
  std::cout << "G:" << group_number << "ADD_TRACK:LPE->REMOVE_TRACKS" << std::endl;
#endif
  // ADD_TRACK -> REMOVE_TRACKS
  gm.SetState(RemoveTracks::getInstance(), tm, group_number, track_number);
}

AddTrack& AddTrack::getInstance() {
  static AddTrack singleton;
  return singleton;
}

/*
 * REMOVE_TRACKS
 */
// Last event received was a play event
void RemoveTracks::enter(GroupManager &gm, TrackManager &tm, uint32_t group_number, uint32_t track_number) {
  gm.GroupRemoveTrack(group_number);
  gm.SetActiveGroup(group_number, tm);
}

void RemoveTracks::exit(GroupManager &gm, TrackManager &tm, uint32_t group_number, uint32_t track_number) {
}

void RemoveTracks::active(GroupManager &gm, TrackManager &tm, uint32_t group_number, uint32_t track_number) {
  gm.RemoveTrackFromGroup(track_number, group_number);
}

  // Event State Transitions
void RemoveTracks::handle_down_event(GroupManager &gm, TrackManager &tm, uint32_t group_number, uint32_t track_number) {
  if (gm.GetActiveGroup() == group_number) {
#ifdef DTEST_VERBOSE_GM
    std::cout << "G:" << group_number << "REMOVE_TRACKS:HDE->NOT_ACTIVE" << std::endl;
#endif
    gm.SetState(NotActive::getInstance(), tm, group_number, track_number);
  } else {
#ifdef DTEST_VERBOSE_GM
    std::cout << "G:" << group_number << ":REMOVE_TRACKS:HDE->ACTIVE_NEW_GROUP" << std::endl;
#endif
    // If the group number is different, re-enter the Active state
    gm.SetState(Active::getInstance(), tm, group_number, track_number);
  }
}

void RemoveTracks::handle_double_down_event(GroupManager &gm, TrackManager &tm, uint32_t group_number, uint32_t track_number) {
#ifdef DTEST_VERBOSE_GM
  std::cout << "G:" << group_number << "REMOVE_TRACKS:DDE" << std::endl;
#endif
}

void RemoveTracks::handle_short_pulse_event(GroupManager &gm, TrackManager &tm, uint32_t group_number, uint32_t track_number) {
#ifdef DTEST_VERBOSE_GM
  std::cout << "G:" << group_number << "REMOVE_TRACKS:SPE" << std::endl;
#endif
}

void RemoveTracks::handle_long_pulse_event(GroupManager &gm, TrackManager &tm, uint32_t group_number, uint32_t track_number) {
#ifdef DTEST_VERBOSE_GM
  std::cout << "G:" << group_number << "REMOVE_TRACKS:LPE" << std::endl;
#endif
}

RemoveTracks& RemoveTracks::getInstance() {
  static RemoveTracks singleton;
  return singleton;
}

