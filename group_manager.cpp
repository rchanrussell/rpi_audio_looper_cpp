#include "group_manager.h"
#include "group_manager_states.h"

// Default Constructor - set all data to zero
GroupManager::GroupManager() {
  active_group = MAX_GROUP_COUNT; // This forces user to set the active group
  // and it prevents group 0 from being ignored by SetActiveGroup
  current_state = &NotActive::getInstance();
  for (auto &g : groups) {
    g = 0;
  }
  for (auto &i : group_master_end_index) {
    i = 0;
  }
  output_i2c = nullptr;
}

void GroupManager::AddTrackToGroup(uint32_t track_number, uint8_t group_number) {
  groups.at(group_number) |= 0x1 << track_number;
#ifdef DTEST_VERBOSE //_GM
  std::cout << "GM::ATTG" << std::endl;
#endif
  if (output_i2c != nullptr) {
    // This will kickstart a detached thread in the output_i2c object
    // to prevent slowing the main app down
    output_i2c->SignalTracksInGroup(
                groups.at(active_group),
		0, 0, 0); // Don't update everything here, just the group LED
  }
}

void GroupManager::GroupAddTrack(uint8_t group_number) {
  if (output_i2c != nullptr) {
    output_i2c->SignalGroupAddTrack(group_number);
  }
}

void GroupManager::GroupRemoveTrack(uint8_t group_number) {
  if (output_i2c != nullptr) {
    output_i2c->SignalGroupRemoveTrack(group_number);
  }
}

void GroupManager::RemoveTrackFromGroup(uint32_t track_number, uint8_t group_number) {
  groups.at(group_number) &= ~(0x1 << track_number);
  if (output_i2c != nullptr) {
    // This will kickstart a detached thread in the output_i2c object
    // to prevent slowing the main app down
    output_i2c->SignalTracksInGroup(
                groups.at(active_group),
		0, 0, 0); // Don't update everything here, just the group LED
  }
}
#ifdef DTEST_GM
void GroupManager::SetGroupMasterEndIndex(uint32_t end, uint8_t group_number) {
  group_master_end_index.at(group_number) = end;
}
#endif

// Don't turn off LEDs here, as this is intended to be a same group active -> not active
// transition for a pause in the audio, not hiding which tracks are members of the
// active group
void GroupManager::SilenceAllTracks(TrackManager &tm) {
#ifdef DTEST_VERBOSE //_GM
  std::cout << "GM::SAT" << std::endl;
#endif
  tm.HandleMuteUnmuteTracks(0xFFFF);
}

void GroupManager::GroupInactive(uint8_t group_number) {
  if (output_i2c != nullptr) {
    output_i2c->SignalGroupInactive(group_number);
  }
}

void GroupManager::UnmuteActiveGroupTracks(TrackManager &tm) {
#ifdef DTEST_VERBOSE //_GM
  std::cout << "GM::UAGT" << std::endl;
#endif
  tm.HandleMuteUnmuteTracks(~(groups.at(active_group)));
  if (output_i2c != nullptr) {
    // This will kickstart a detached thread in the output_i2c object
    // to prevent slowing the main app down
    output_i2c->SignalTracksInGroup(
                groups.at(active_group),
                tm.GetTracksInPlayback(),
		tm.GetTracksInMute(),
		tm.GetTracksOff());
  }
}
void GroupManager::GroupActive(uint8_t new_group) {
  if (output_i2c != nullptr) {
    if (IsGroupEmpty(new_group)) {
      output_i2c->SignalGroupActiveEmpty(new_group);
    } else {
      output_i2c->SignalGroupActiveWithTrack(new_group);
    }
  }
}
void GroupManager::SetActiveGroup(uint8_t new_group, TrackManager &tm) {
#ifdef DTEST_VERBOSE //_GM
  std::cout << "GM::SAG from " << unsigned(active_group) << " to  " << unsigned(new_group) << std::endl;
#endif

  if (new_group == active_group) {
#ifdef DTEST_VERBOSE_GM
  std::cout << "GM::SAG return early, grp " << unsigned(new_group) << std::endl;
#endif
    return;
  }

  // group change should reset the master_current_index to 0
  // what happens if one group is shorter in loop than another?
  // group should store master_end_index per group!
  if (active_group != MAX_GROUP_COUNT) {
    group_master_end_index.at(active_group) = tm.GetMasterEndIndex();
  }
  active_group = new_group;
  // Load group's master end index
  tm.SetMasterEndIndex(group_master_end_index.at(active_group));
  tm.SetMasterCurrentIndex(0);

#ifdef DTEST_VERBOSE_GM
  std::cout << "GM::SAG new active grp " << unsigned(new_group) << std::endl;
  std::cout << "GM::SAGE new active grp master end index " << tm.GetMasterEndIndex() << std::endl;
#endif
  UnmuteActiveGroupTracks(tm);
#ifdef DTEST_VERBOSE_GM
  std::cout << "GM::SAG exiting active_grp " << unsigned(active_group) << std::endl;
#endif
}

uint8_t GroupManager::GetActiveGroup() {
  return active_group;
}

void GroupManager::ResetActiveGroupToNone() {
  active_group = MAX_GROUP_COUNT;
}

void GroupManager::DisplayGroups() {
  for (auto &g : groups) {
    std::cout << "0x" << std::hex << g << " ";
  }
  std::cout << std::endl;
}

void GroupManager::HandleDownEvent(TrackManager &tm, uint32_t group_number, uint32_t track_number) {
  current_state->handle_down_event(*this, tm, group_number, track_number);
}

void GroupManager::HandleDoubleDownEvent(TrackManager &tm, uint32_t group_number, uint32_t track_number) {
  current_state->handle_double_down_event(*this, tm, group_number, track_number);
}

void GroupManager::HandleShortPulseEvent(TrackManager &tm, uint32_t group_number, uint32_t track_number) {
  current_state->handle_short_pulse_event(*this, tm, group_number, track_number);
}

void GroupManager::HandleLongPulseEvent(TrackManager &tm, uint32_t group_number, uint32_t track_number) {
  current_state->handle_long_pulse_event(*this, tm, group_number, track_number);
}

void GroupManager::SetState(GroupManagerState &new_state, TrackManager &tm, uint32_t group_number, uint32_t track_number) {
  current_state->exit(*this, tm, group_number, track_number);
  current_state = &new_state;
  current_state->enter(*this, tm, group_number, track_number);
}

void GroupManager::StateProcess(TrackManager &tm, uint32_t group_number, uint32_t track_number) {
  current_state->active(*this, tm, group_number, track_number);
}

bool GroupManager::IsStateNotActive() {
  return static_cast<void*>(current_state) == static_cast<void*>(&NotActive::getInstance());
}

bool GroupManager::IsStateActive() {
  return static_cast<void*>(current_state) == static_cast<void*>(&Active::getInstance());
}

bool GroupManager::IsStateAddTrack() {
  return static_cast<void*>(current_state) == static_cast<void*>(&AddTrack::getInstance());
}

bool GroupManager::IsStateRemoveTracks() {
  return static_cast<void*>(current_state) == static_cast<void*>(&RemoveTracks::getInstance());
}

void GroupManager::SetOutputI2CPtr(OutputI2C* obj) {
  output_i2c = obj;
}

bool GroupManager::IsGroupEmpty(uint8_t group_number) {
  if (group_number == MAX_GROUP_COUNT) { return false; }
  return groups.at(group_number) == 0;
}

bool GroupManager::AreGroupTracksOff(uint8_t group_number, TrackManager &tm) {
  if (group_number == MAX_GROUP_COUNT) { return false; }
  uint16_t all_off_tracks = tm.GetTracksOff();
  uint16_t tracks_in_group = groups.at(group_number);
  tracks_in_group &= all_off_tracks;
  return tracks_in_group == groups.at(group_number);
}

uint16_t GroupManager::GetTracksInGroup(uint8_t group_number) {
  return groups.at(group_number);
}

bool GroupManager::IsTrackMemberOfGroup(uint32_t track, uint8_t group) {
  if (group > MAX_GROUP_COUNT) { return false; }
  // If no groups, active group will be MAX_GROUP_COUNT
  // We want it to work so always return true - so there's no blocking
  if (group == MAX_GROUP_COUNT) { return true; }
  return groups.at(group) & (0x1 << track);
}
