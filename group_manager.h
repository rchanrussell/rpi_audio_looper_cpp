#ifndef GROUP_MANAGER_H
#define GROUP_MANAGER_H

#include <array>
#include <iostream>
#include <iterator>

#include "util.h"
#include "track_manager.h"
#include "group_manager_state.h"

/*
enum SystemEvents
{
  SYSTEM_EVENT_ADD_TRACK_TO_GROUP,        // Adds a track to a group - nothing more - track # & group # required
  SYSTEM_EVENT_REMOVE_TRACK_FROM_GROUP,   // Removes track from a group - track # & group # required
  SYSTEM_EVENT_SET_ACTIVE_GROUP,          // Sets the currently active group - group # required
};
*/

class GroupManagerState;

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

  GroupManagerState* current_state;

  public:
  // Member variables

  // Member Functions
  GroupManager();

  // State Machine Section
  void SetState(GroupManagerState &new_state, TrackManager &tm, uint32_t group_number, uint32_t track_number);
  inline GroupManagerState& GetCurrentState() const { return *current_state; }
  // Adding tracks to active group
  void StateProcess(TrackManager &tm, uint32_t group_number, uint32_t track_number);

  //
  bool IsStateNotActive();
  bool IsStateActive();
  bool IsStateAddTrack();
  bool IsStateRemoveAllTracks();

  // Call the current state's versions
  void HandleDownEvent(TrackManager &tm, uint32_t group_number, uint32_t track_number);
  void HandleDoubleDownEvent(TrackManager &tm, uint32_t group_number, uint32_t track_number);
  void HandleShortPulseEvent(TrackManager &tm, uint32_t group_number, uint32_t track_number);
  void HandleLongPulseEvent(TrackManager &tm, uint32_t group_number, uint32_t track_number);

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
