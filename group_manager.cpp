#include "group_manager.h"

// Default Constructor - set all data to zero
GroupManager::GroupManager() {
  std::cout << "GroupManager: DF called" << std::endl;
  active_group = 0;
}

void GroupManager::AddTrackToGroup(uint32_t track_number, uint8_t group_number) {
  groups.at(group_number) |= 0x1 << track_number;
}

void GroupManager::RemoveTrackFromGroup(uint32_t track_number, uint8_t group_number) {
  groups.at(group_number) &= ~(0x1 << track_number);
}

void GroupManager::SetActiveGroup(uint8_t new_group, TrackManager &tm) {
  if (new_group == active_group) {
    return;
  }
  active_group = new_group;
  uint16_t track_mute_unmute = 0xFFFF; // default - mute every track
  int8_t group_index = 0;
  for (auto &g : groups) {
    if (group_index == new_group) {
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
}

uint8_t GroupManager::GetActiveGroup() {
  return active_group;
}

void GroupManager::DisplayGroups() {
  for (auto &g : groups) {
    std::cout << "0x" << std::hex << g << " ";
  }
  std::cout << std::endl;
}
