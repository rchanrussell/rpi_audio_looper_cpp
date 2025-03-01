#ifndef GROUP_MANAGER_STATES_H
#define GROUP_MANAGER_STATES_H

#include "group_manager_state.h"
#include "group_manager.h"

/*
 * Always, any state, the mixdown is performed and data is
 * copied to the output buffer
 */

// All groups are off - system is in idle
class NotActive : public GroupManagerState {
  private:
  NotActive() {};
  NotActive(const NotActive& other);
  NotActive& operator=(const NotActive& other);

  public:
  // State Specific Methods
  void enter(GroupManager &gm, TrackManager &tm, uint32_t group_number, uint32_t track_number);
  void exit(GroupManager &gm, TrackManager &tm, uint32_t group_number, uint32_t track_number);
  void active(GroupManager &gm, TrackManager &tm, uint32_t group_number, uint32_t track_number);

  // Event Specific Methods
  void handle_down_event(GroupManager &gm, TrackManager &tm, uint32_t group_number, uint32_t track_number);
  void handle_double_down_event(GroupManager &gm, TrackManager &tm, uint32_t group_number, uint32_t track_number);
  void handle_short_pulse_event(GroupManager &gm, TrackManager &tm, uint32_t group_number, uint32_t track_number);
  void handle_long_pulse_event(GroupManager &gm, TrackManager &tm, uint32_t group_number, uint32_t track_number);

  static NotActive& getInstance();
};

// Last event received was a record event - copy data from source
class Active : public GroupManagerState {
  private:
  Active() {};
  Active(const Active& other);
  Active& operator=(const Active& other);

  public:
  // State Specific Methods
  void enter(GroupManager &gm, TrackManager &tm, uint32_t group_number, uint32_t track_number);
  void exit(GroupManager &gm, TrackManager &tm, uint32_t group_number, uint32_t track_number);
  void active(GroupManager &gm, TrackManager &tm, uint32_t group_number, uint32_t track_number);

  // Event Specific Methods
  void handle_down_event(GroupManager &gm, TrackManager &tm, uint32_t group_number, uint32_t track_number);
  void handle_double_down_event(GroupManager &gm, TrackManager &tm, uint32_t group_number, uint32_t track_number);
  void handle_short_pulse_event(GroupManager &gm, TrackManager &tm, uint32_t group_number, uint32_t track_number);
  void handle_long_pulse_event(GroupManager &gm, TrackManager &tm, uint32_t group_number, uint32_t track_number);

  static Active& getInstance();
};

// Last event received was a overdub event - copy data from source
class AddTrack : public GroupManagerState {
  private:
  AddTrack() {};
  AddTrack(const Active& other);
  AddTrack& operator=(const Active& other);

  public:
  // State Specific Methods
  void enter(GroupManager &gm, TrackManager &tm, uint32_t group_number, uint32_t track_number);
  void exit(GroupManager &gm, TrackManager &tm, uint32_t group_number, uint32_t track_number);
  void active(GroupManager &gm, TrackManager &tm, uint32_t group_number, uint32_t track_number);

  // Event Specific Methods
  void handle_down_event(GroupManager &gm, TrackManager &tm, uint32_t group_number, uint32_t track_number);
  void handle_double_down_event(GroupManager &gm, TrackManager &tm, uint32_t group_number, uint32_t track_number);
  void handle_short_pulse_event(GroupManager &gm, TrackManager &tm, uint32_t group_number, uint32_t track_number);
  void handle_long_pulse_event(GroupManager &gm, TrackManager &tm, uint32_t group_number, uint32_t track_number);

  static AddTrack& getInstance();
};

// Last event received was a play event
class RemoveAllTracks : public GroupManagerState {
  private:
  RemoveAllTracks() {};
  RemoveAllTracks(const Active& other);
  RemoveAllTracks& operator=(const Active& other);

  public:
  // State Specific Methods
  void enter(GroupManager &gm, TrackManager &tm, uint32_t group_number, uint32_t track_number);
  void exit(GroupManager &gm, TrackManager &tm, uint32_t group_number, uint32_t track_number);
  void active(GroupManager &gm, TrackManager &tm, uint32_t group_number, uint32_t track_number);

  // Event Specific Methods
  void handle_down_event(GroupManager &gm, TrackManager &tm, uint32_t group_number, uint32_t track_number);
  void handle_double_down_event(GroupManager &gm, TrackManager &tm, uint32_t group_number, uint32_t track_number);
  void handle_short_pulse_event(GroupManager &gm, TrackManager &tm, uint32_t group_number, uint32_t track_number);
  void handle_long_pulse_event(GroupManager &gm, TrackManager &tm, uint32_t group_number, uint32_t track_number);

  static RemoveAllTracks& getInstance();
};

#endif // GROUP_MANAGER_STATES_H
