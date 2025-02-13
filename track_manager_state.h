#ifndef TRACK_MANAGER_STATE_H
#define TRACK_MANAGER_STATE_H

#include "track_manager.h"

class TrackManager;

class TrackManagerState {
  public:
  // State Specific Methods
  virtual void enter(TrackManager &tm, uint32_t track_number) = 0;
  virtual void exit(TrackManager &tm, uint32_t track_number) = 0;
  virtual void active(TrackManager &tm, uint32_t track_number) = 0;

  // Event Specific Methods
  virtual void handle_down_event(TrackManager &tm, uint32_t track_number) = 0;
  virtual void handle_double_down_event(TrackManager &tm, uint32_t track_number) = 0;
  virtual void handle_short_pulse_event(TrackManager &tm, uint32_t track_number) = 0;
  virtual void handle_long_pulse_event(TrackManager &tm, uint32_t track_number) = 0;

  virtual ~TrackManagerState() {};
  
};


#endif // TRACK_MANAGER_STATE_H
