#ifndef GROUP_MANAGER_STATE_H
#define GROUP_MANAGER_STATE_H

#include "group_manager.h"

class GroupManager;

class GroupManagerState {
  public:
  // State Specific Methods
  virtual void enter(GroupManager &gm, TrackManager &tm, uint32_t group_number, uint32_t track_number) = 0;
  virtual void exit(GroupManager &gm, TrackManager &tm, uint32_t group_number, uint32_t track_number) = 0;
  virtual void active(GroupManager &gm, TrackManager &tm, uint32_t group_number, uint32_t track_number) = 0;

  // Event Specific Methods
  virtual void handle_down_event(GroupManager &gm, TrackManager &tm, uint32_t group_number, uint32_t track_number) = 0;
  virtual void handle_double_down_event(GroupManager &gm, TrackManager &tm, uint32_t group_number, uint32_t track_number) = 0;
  virtual void handle_short_pulse_event(GroupManager &gm, TrackManager &tm, uint32_t group_number, uint32_t track_number) = 0;
  virtual void handle_long_pulse_event(GroupManager &gm, TrackManager &tm, uint32_t group_number, uint32_t track_number) = 0;

  virtual ~GroupManagerState() {};
  
};


#endif // GROUP_MANAGER_STATE_H
