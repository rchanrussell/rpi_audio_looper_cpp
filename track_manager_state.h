#ifndef TRACK_MANAGER_STATE_H
#define TRACK_MANAGER_STATE_H

#include "track_manager.h"

class TrackManager;

class TrackManagerState {
  public:
  // State Specific Methods
  virtual void Enter(TrackManager &tm, uint32_t track_number) = 0;
  virtual void Exit(TrackManager &tm, uint32_t track_number) = 0;
  virtual void Active(TrackManager &tm, uint32_t track_number) = 0;

  // Event Specific Methods
  virtual void DownEvent(TrackManager &tm, uint32_t track_number) = 0;
  virtual void DoubleDownEvent(TrackManager &tm, uint32_t track_number) = 0;
  virtual void ShortPulseEvent(TrackManager &tm, uint32_t track_number) = 0;
  virtual void LongPulseEvent(TrackManager &tm, uint32_t track_number) = 0;

  virtual ~TrackManagerState() {};
  
};


#endif // TRACK_MANAGER_STATE_H
