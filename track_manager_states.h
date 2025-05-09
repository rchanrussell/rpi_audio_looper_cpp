#ifndef TRACK_MANAGER_STATES_H
#define TRACK_MANAGER_STATES_H

#include "track_manager_state.h"
#include "track_manager.h"

/*
 * Always, any state, the mixdown is performed and data is
 * copied to the output buffer
 */

// All tracks are off - system is in idle
class Off : public TrackManagerState {
  private:
  Off() {};
  Off(const Off& other);
  Off& operator=(const Off& other);

  public:
  // State Specific Methods
  void Enter(TrackManager &tm, uint32_t track_number);
  void Exit(TrackManager &tm, uint32_t track_number);
  void Active(TrackManager &tm, uint32_t track_number);

  // Event Specific Methods
  void DownEvent(TrackManager &tm, uint32_t track_number);
  void DoubleDownEvent(TrackManager &tm, uint32_t track_number);
  void ShortPulseEvent(TrackManager &tm, uint32_t track_number);
  void LongPulseEvent(TrackManager &tm, uint32_t track_number);

  static Off& getInstance();
};

// Last event received was a record event - copy data from source
class Record : public TrackManagerState {
  private:
  Record() {};
  Record(const Record& other);
  Record& operator=(const Record& other);

  public:
  // State Specific Methods
  void Enter(TrackManager &tm, uint32_t track_number);
  void Exit(TrackManager &tm, uint32_t track_number);
  void Active(TrackManager &tm, uint32_t track_number);

  // Event Specific Methods
  void DownEvent(TrackManager &tm, uint32_t track_number);
  void DoubleDownEvent(TrackManager &tm, uint32_t track_number);
  void ShortPulseEvent(TrackManager &tm, uint32_t track_number);
  void LongPulseEvent(TrackManager &tm, uint32_t track_number);

  static Record& getInstance();
};

// Last event received was a overdub event - copy data from source
class Overdub : public TrackManagerState {
  private:
  Overdub() {};
  Overdub(const Record& other);
  Overdub& operator=(const Record& other);

  public:
  // State Specific Methods
  void Enter(TrackManager &tm, uint32_t track_number);
  void Exit(TrackManager &tm, uint32_t track_number);
  void Active(TrackManager &tm, uint32_t track_number);

  // Event Specific Methods
  void DownEvent(TrackManager &tm, uint32_t track_number);
  void DoubleDownEvent(TrackManager &tm, uint32_t track_number);
  void ShortPulseEvent(TrackManager &tm, uint32_t track_number);
  void LongPulseEvent(TrackManager &tm, uint32_t track_number);

  static Overdub& getInstance();
};

// Last event received was a play event
class Play : public TrackManagerState {
  private:
  Play() {};
  Play(const Record& other);
  Play& operator=(const Record& other);

  public:
  // State Specific Methods
  void Enter(TrackManager &tm, uint32_t track_number);
  void Exit(TrackManager &tm, uint32_t track_number);
  void Active(TrackManager &tm, uint32_t track_number);

  // Event Specific Methods
  void DownEvent(TrackManager &tm, uint32_t track_number);
  void DoubleDownEvent(TrackManager &tm, uint32_t track_number);
  void ShortPulseEvent(TrackManager &tm, uint32_t track_number);
  void LongPulseEvent(TrackManager &tm, uint32_t track_number);

  static Play& getInstance();
};

// Last event received was a repeat event
class Repeat : public TrackManagerState {
  private:
  Repeat() {};
  Repeat(const Record& other);
  Repeat& operator=(const Record& other);

  public:
  // State Specific Methods
  void Enter(TrackManager &tm, uint32_t track_number);
  void Exit(TrackManager &tm, uint32_t track_number);
  void Active(TrackManager &tm, uint32_t track_number);

  // Event Specific Methods
  void DownEvent(TrackManager &tm, uint32_t track_number);
  void DoubleDownEvent(TrackManager &tm, uint32_t track_number);
  void ShortPulseEvent(TrackManager &tm, uint32_t track_number);
  void LongPulseEvent(TrackManager &tm, uint32_t track_number);

  static Repeat& getInstance();
};

// Last event received was a mute event
class Mute : public TrackManagerState {
  private:
  Mute() {};
  Mute(const Record& other);
  Mute& operator=(const Record& other);

  public:
  // State Specific Methods
  void Enter(TrackManager &tm, uint32_t track_number);
  void Exit(TrackManager &tm, uint32_t track_number);
  void Active(TrackManager &tm, uint32_t track_number);

  // Event Specific Methods
  void DownEvent(TrackManager &tm, uint32_t track_number);
  void DoubleDownEvent(TrackManager &tm, uint32_t track_number);
  void ShortPulseEvent(TrackManager &tm, uint32_t track_number);
  void LongPulseEvent(TrackManager &tm, uint32_t track_number);

  static Mute& getInstance();
};


#endif // TRACK_MANAGER_STATES_H
