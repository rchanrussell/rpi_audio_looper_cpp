#ifndef GROUP_MANAGER_H
#define GROUP_MANAGER_H

#include <array>
#include <iostream>
#include <iterator>
#include "track_manager.h"

#define MAX_GROUP_COUNT 8

/*
enum SystemEvents
{
  SYSTEM_EVENT_ADD_TRACK_TO_GROUP,        // Adds a track to a group - nothing more - track # & group # required
  SYSTEM_EVENT_REMOVE_TRACK_FROM_GROUP,   // Removes track from a group - track # & group # required
  SYSTEM_EVENT_SET_ACTIVE_GROUP,          // Sets the currently active group - group # required
};
*/

// Tracks can be on multiple groups - useful for repeating rhythmic pattern

class GroupManager {
  /*
   * g: 0 t:0-15
   * g: 1 t:0-15
   * ...
   * g: 7 t:0-15
   */
  std::array<uint16_t, MAX_GROUP_COUNT> groups;
  std::array<uint32_t, MAX_GROUP_COUNT> group_master_end_index;
  uint8_t active_group;

  public:
  // Member variables

  // Member Functions
  GroupManager();

  // Local storage only - meant for configuration
  void AddTrackToGroup(uint32_t track_number, uint8_t group_number);
  void RemoveTrackFromGroup(uint32_t track_number, uint8_t group_number);
  void SetActiveGroupToPlay(TrackManager &tm);
#ifdef DTEST_GM
  void SetGroupMasterEndIndex(uint32_t end, uint8_t group_number);
#endif

  // Calls TrackManager's HandleMuteUnmuteTracks - convert dual dim array to uint16_t
  // 0 - unmute, 1 - mute
  void SetActiveGroup(uint8_t group_number, TrackManager &tm);
  uint8_t GetActiveGroup();
  void ResetActiveGroupToNone();
  void DisplayGroups();

};
#endif // GROUP_MANAGER_H
