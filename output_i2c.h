#ifndef OUTPUT_I2C_H
#define OUTPUT_I2C_H

#include <vector>

#define EXP0_ADDR 0x3E
#define EXP1_ADDR 0x3F
#define EXP2_ADDR 0x70
#define EXP3_ADDR 0x71
#define LEDS_PER_TRACK 3

class OutputI2C {
  // Variables
  int i2c_red_fd;
  int i2c_green_fd;
  int i2c_yellow_fd;
  int i2c_dev3_fd;

  // Methods
  bool InitializeExpander(int fd);
  bool ConfigureLEDDriver(int fd);
  bool SetLEDOn(int fd, uint16_t led);
  bool SetLEDOff(int fd, uint16_t led);
  bool SetLEDBlink(int fd, uint16_t led);
  bool SetLEDIntensity(int fd, uint16_t led, bool set_max);
  int TrackToDevFd(uint32_t track);

  // detached threads
  void SignalRecord(uint32_t track);
  void SignalPlayback(uint32_t track);
  void SignalMuted(uint32_t track);
  void SignalOff(uint32_t track);
  void SignalInGroup(uint32_t track);
  void SignalNotInGroup(uint32_t track);
  void SignalTracksInGroupThread(
       uint16_t tracks_in_group,
       uint16_t tracks_in_playback,
       uint16_t tracks_in_mute,
       uint16_t tracks_off);
  void SignalGroupActiveWithTrackThread(uint8_t group_number);
  void SignalGroupAddTrackThread(uint8_t group_number);
  void SignalGroupActiveEmptyThread(uint8_t group_number);
  void SignalGroupRemoveTrackThread(uint8_t group_number);
  void SignalGroupInactiveThread(uint8_t group_number);

  public:

  OutputI2C();
  bool InitializeWiringPiI2C();
  // solid red only - recording and overdub
  void SignalTrackRecording(uint32_t track);
  // solid green only - playback and repeat
  void SignalTrackPlayback(uint32_t track);
  // blink green only - don't set when switching groups
  void SignalTrackMuted(uint32_t track);
  // turn off LEDs - off state
  void SignalTrackOff(uint32_t track);
  // yellow LED on if track bit, off if not
  void SignalTracksInGroup(
       uint16_t tracks_in_group,
       uint16_t tracks_in_playback,
       uint16_t tracks_in_mute,
       uint16_t tracks_off);
  void SignalGroupActiveWithTrack(uint8_t group_number);
  void SignalGroupAddTrack(uint8_t group_number);
  void SignalGroupActiveEmpty(uint8_t group_number);
  void SignalGroupRemoveTrack(uint8_t group_number);
  void SignalGroupInactive(uint8_t group_number);

};

#endif //OUTPUT_I2C_H
