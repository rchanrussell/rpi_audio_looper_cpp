#include "group_manager.h"
#include "group_manager_states.h"

// Default Constructor - set all data to zero
GroupManager::GroupManager() {
  std::cout << "GroupManager: DF called" << std::endl;
  active_group = MAX_GROUP_COUNT; // This forces user to set the active group
  // and it prevents group 0 from being ignored by SetActiveGroup
  current_state = &NotActive::getInstance();
  for (auto &g : groups) {
    g = 0;
  }
  for (auto &i : group_master_end_index) {
    i = 0;
  }
}

void GroupManager::AddTrackToGroup(uint32_t track_number, uint8_t group_number) {
  groups.at(group_number) |= 0x1 << track_number;
}

void GroupManager::RemoveTrackFromGroup(uint32_t track_number, uint8_t group_number) {
  groups.at(group_number) &= ~(0x1 << track_number);
}
#ifdef DTEST_GM
void GroupManager::SetGroupMasterEndIndex(uint32_t end, uint8_t group_number) {
  group_master_end_index.at(group_number) = end;
}
#endif

void GroupManager::SetActiveGroupToPlay(TrackManager &tm) {
  int8_t group_index = 0;
  for (auto &g : groups) {
    if (group_index == active_group) {
#ifdef DTEST_VERBOSE
      std::cout << "GM::SAGTP - grp == active_group: " << unsigned(group_index) << std::endl;
      std::cout << std::hex << "0x" << g << std::endl;
#endif
      for (uint8_t track_index = 0; track_index < MAX_TRACK_COUNT; track_index++) {
        if (0x1 << track_index & g) {
          // track is member of group -- set it to play
#ifdef DTEST_VERBOSE
          std::cout << "set to play " << unsigned(track_index) << ",";
#endif
          // Don't force to playback, what if was off?
          tm.HandleStateChange_Playback(track_index);
        }
      }
    }
    group_index++;
  }
#ifdef DTEST_VERBOSE
  std::cout << std::endl;
#endif
}

void GroupManager::SetActiveGroup(uint8_t new_group, TrackManager &tm) {
#ifdef DTEST_VERBOSE
  std::cout << "GM::SAG from " << unsigned(active_group) << " to  " << unsigned(new_group) << std::endl;
#endif
  if (new_group == active_group) {
#ifdef DTEST_VERBOSE
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

#ifdef DTEST_VERBOSE
  std::cout << "GM::SAG new active grp " << unsigned(new_group) << std::endl;
  std::cout << "GM::SAGE new active grp master end index " << tm.GetMasterEndIndex() << std::endl;
#endif
  uint16_t track_mute_unmute = 0xFFFF; // default - mute every track
  int8_t group_index = 0;
  for (auto &g : groups) {
    if (group_index == new_group) {
#ifdef DTEST_VERBOSE
      std::cout << "GM::SAG going through group " << unsigned(group_index) << std::endl;
#endif
      for (uint8_t track_index = 0; track_index < MAX_TRACK_COUNT; track_index++) {
        if (0x1 << track_index & g) {
          // track is member of group unmute
          track_mute_unmute &= ~(0x1 << track_index);
        }
      }
      tm.HandleMuteUnmuteTracks(track_mute_unmute);
    }
    group_index++;
  }
#ifdef DTEST_VERBOSE
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

bool GroupManager::IsStateRemoveAllTracks() {
  return static_cast<void*>(current_state) == static_cast<void*>(&RemoveAllTracks::getInstance());
}
