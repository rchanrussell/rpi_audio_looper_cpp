#include <iostream>
#include <string>
#include <unistd.h>
#include <sys/time.h>
#include <chrono>
#include <thread>
#include <wiringPi.h>
#include <wiringPiI2C.h>
#include "output_i2c.h"

struct I2CAddrValue {
  uint8_t addr;
  uint16_t value;
};

// Disable inputs, pullups, enable open drain, set direction as output
// set outputs to high - LED off if current sinking
std::vector<struct I2CAddrValue> i2c_sx1509_led_config = {
  {0x00, 0xFFFF}, {0x06, 0x0000},
  {0x0A, 0xFFFF}, {0x0E, 0x0000},
  {0x10, 0x0000}
};

// Not a consistent offset, as some outputs have rise and fall registers, others
// do not
std::vector<uint8_t> i2c_sx1509_led_ton = {
  0x29, 0x2C, 0x2F, 0x32, 0x35, 0x3A, 0x3F, 0x44,
  0x49, 0x4C, 0x4F, 0x52, 0x55, 0x5A, 0x5F, 0x64
};

std::vector<uint8_t> i2c_sx1509_led_toff = {
  0x2B, 0x2E, 0x31, 0x34, 0x37, 0x3C, 0x41, 0x46,
  0x4B, 0x4E, 0x51, 0x54, 0x57, 0x5C, 0x61, 0x66
};
std::vector<uint8_t> i2c_sx1509_led_intensity = {
  0x2A, 0x2D, 0x30, 0x33, 0x36, 0x3B, 0x40, 0x45,
  0x4A, 0x4D, 0x50, 0x53, 0x56, 0x5B, 0x60, 0x65
};

OutputI2C::OutputI2C() {
  i2c_dev0_fd = -1;
  i2c_dev1_fd = -1;
  i2c_dev2_fd = -1;
  i2c_dev3_fd = -1;
}

bool OutputI2C::InitializeWiringPiI2C() {
  i2c_dev0_fd = wiringPiI2CSetup(EXP0_ADDR);
  if (i2c_dev0_fd < 0){
    std::cout << "Error, device does not exist" << std::endl;
  }
#if 0 // wait until the expanders are connected
  i2c_dev1_fd = wiringPiI2CSetup(EXP1_ADDR);
  if (i2c_dev1_fd < 0){
    std::cout << "Error, device does not exist" << std::endl;
  }

  i2c_dev2_fd = wiringPiI2CSetup(EXP2_ADDR);
  if (i2c_dev2_fd < 0){
    std::cout << "Error, device does not exist" << std::endl;
  }

  i2c_dev3_fd = wiringPiI2CSetup(EXP3_ADDR);
  if (i2c_dev3_fd < 0){
    std::cout << "Error, device does not exist" << std::endl;
  }
#endif

  bool at_least_one_dev = false;
  std::cout << "Initializing expander at " << std::hex << EXP0_ADDR << std::endl;
  if (i2c_dev0_fd >= 0) {
    if (!InitializeExpander(i2c_dev0_fd)) {
      std::cout << "Error: failed to initialize device" << std::endl;
    } else {
      at_least_one_dev = true;
    }
  }

  std::cout << "Initializing expander at " << std::hex << EXP1_ADDR << std::endl;
  if (i2c_dev1_fd >= 0) {
    if (!InitializeExpander(i2c_dev1_fd)) {
      std::cout << "Error: failed to initialize device" << std::endl;
    } else {
      at_least_one_dev = true;
    }
  }

  std::cout << "Initializing expander at " << std::hex << EXP2_ADDR << std::endl;
  if (i2c_dev2_fd >= 0) {
    if (!InitializeExpander(i2c_dev2_fd)) {
      std::cout << "Error: failed to initialize device" << std::endl;
    } else {
      at_least_one_dev = true;
    }
  }

  std::cout << "Initializing expander at " << std::hex << EXP3_ADDR << std::endl;
  if (i2c_dev3_fd >= 0) {
    if (!InitializeExpander(i2c_dev3_fd)) {
      std::cout << "Error: failed to initialize device" << std::endl;
    } else {
      at_least_one_dev = true;
    }
  }

  return at_least_one_dev;
}

// Call for each device, using it's own file descriptor
bool OutputI2C::InitializeExpander(int fd) {
  std::cout << "IE " << std::endl;
  for (uint8_t idx = 0; idx < i2c_sx1509_led_config.size(); idx++) { 
    if (wiringPiI2CWriteReg16(fd,
                             i2c_sx1509_led_config.at(idx).addr,
                             i2c_sx1509_led_config.at(idx).value) < 0) {
      std::cout << "Error configuring I2C exp " << fd << ", init step " << idx << std::endl;
      return false;
    }
  }
  return ConfigureLEDDriver(fd);
}

// Specific to SX1509
bool OutputI2C::ConfigureLEDDriver(int fd) {
  // Enable the oscillator
  if (wiringPiI2CWriteReg8(fd, 0x1E, 0x58) < 0) {
    std::cout << "Error I2C enable clock divider" << std::endl;
    return false;
  }
  // Configure the LED driver clock
  // 101b 1000 0x58
  uint8_t reg = wiringPiI2CReadReg8(fd, 0x1F);
  reg &= ~(0b111 << 4);
  reg |= (0b101 << 4);
  if (wiringPiI2CWriteReg8(fd, 0x1F, reg) < 0) {
    std::cout << "Error I2C config LED clock" << std::endl;
    return false;
  }
  // Enable LED driver
  if (wiringPiI2CWriteReg16(fd, 0x20, 0xFFFF) < 0) {
    std::cout << "Error I2C enable LED driver" << std::endl;
    return false;
  }
  // set all LEDs to off
  for (uint8_t led = 0; led < i2c_sx1509_led_ton.size(); led++) {
    if (!SetLEDOff(fd, led)) {
      return false;
    }
    if (!SetLEDIntensity(fd, led, false)) {
      return false;
    }
  }
  // Set all data bits to zero - driver will start
  wiringPiI2CWriteReg16(fd, 0x10, 0x0000);

  return true;
}

// Only 16 LED per expander
bool OutputI2C::SetLEDOff(int fd, uint16_t led) {
  if (fd < 0) { return false; }
  if (led > 15) { return false; }
  uint8_t addr = i2c_sx1509_led_ton.at(led); 
  if (wiringPiI2CWriteReg8(fd, addr, 0x0) < 0) {
    std::cout << "Error I2C set LED On time to 0" << std::endl;
    return false;
  }
  addr = i2c_sx1509_led_toff.at(led);
  if (wiringPiI2CWriteReg8(fd, addr, 0x0) < 0) {
    std::cout << "Error I2C set LED Off time to 0" << std::endl;
    return false;
  }
  // for LED 15:8 use addr 0x10
  // for LED 7:0 use addr 0x11
  // Then reduce LED to 7:0 value as it's by bit in an 8bit register
  addr = (led > 7) ? 0x10 : 0x11;
  if (led > 7) { led -= 8; }
  uint8_t reg = wiringPiI2CReadReg8(fd, addr);
  reg |= (0x1 << led); // set to turn LED off
  wiringPiI2CWriteReg8(fd, addr, reg);
  return SetLEDIntensity(fd, led, false);
}

bool OutputI2C::SetLEDOn(int fd, uint16_t led) {
  if (fd < 0) { return false; }
  if (led > 15) { return false; }
  uint8_t addr = i2c_sx1509_led_ton.at(led); 
  if (wiringPiI2CWriteReg8(fd, addr, 0x00) < 0) {
    std::cout << "Error I2C set LED On time to 0x00" << std::endl;
    return false;
  }
  addr = i2c_sx1509_led_toff.at(led);
  if (wiringPiI2CWriteReg8(fd, addr, 0x00) < 0) {
    std::cout << "Error I2C set LED Off time to 0" << std::endl;
    return false;
  }
  // for LED 15:8 use addr 0x10
  // for LED 7:0 use addr 0x11
  // Then reduce LED to 7:0 value as it's by bit in an 8bit register
  addr = (led > 7) ? 0x10 : 0x11;
  if (led > 7) { led -= 8; }
  uint8_t reg = wiringPiI2CReadReg8(fd, addr);
  reg &= ~(0x1 << led); // clear to turn LED on
  wiringPiI2CWriteReg8(fd, addr, reg);

  return SetLEDIntensity(fd, led, true);
}

bool OutputI2C::SetLEDIntensity(int fd, uint16_t led, bool set_max) {
  if (fd < 0) { return false; }
  if (led > 15) { return false; }
  uint8_t intensity = set_max ? 0xFF : 0x00;
  uint8_t addr = i2c_sx1509_led_intensity.at(led); 
  if (wiringPiI2CWriteReg8(fd, addr, intensity) < 0) {
    std::cout << "Error I2C set LED intensity to " << intensity << std::endl;
    return false;
  }
  return true;
}

bool OutputI2C::SetLEDBlink(int fd, uint16_t led) {
  if (fd < 0) { return false; }
  if (led > 15) { return false; }
  uint8_t addr = i2c_sx1509_led_ton.at(led);
  if (wiringPiI2CWriteReg8(fd, addr, 0x01) < 0) {
    std::cout << "Error I2C set LED On time to 0x1" << std::endl;
    return false;
  }
  addr = i2c_sx1509_led_toff.at(led);
  if (wiringPiI2CWriteReg8(fd, addr, 0x0F) < 0) {
    std::cout << "Error I2C set LED Off time to 0x0F" << std::endl;
    return false;
  }
  // for LED 15:8 use addr 0x10
  // for LED 7:0 use addr 0x11
  // Then reduce LED to 7:0 value as it's by bit in an 8bit register
  addr = (led > 7) ? 0x10 : 0x11;
  if (led > 7) { led -= 8; }
  uint8_t reg = wiringPiI2CReadReg8(fd, addr);
  reg &= ~(0x1 << led); // clear to turn LED on
  wiringPiI2CWriteReg8(fd, addr, reg);

  return SetLEDIntensity(fd, led, true);
}

void OutputI2C::SignalRecord(int fd, uint32_t track) {
  if (track == 15) {
    SetLEDOn(i2c_dev0_fd, 15);
    SetLEDOff(i2c_dev1_fd, 15);
  } else {
    uint16_t led = track * LEDS_PER_TRACK;
    if (led >= 15 && led < 30) { led -= 15; }
    if (led >= 30) { led -= 30; }
    SetLEDOn(fd, led);
    SetLEDOff(fd, led + 1);
  }
}

void OutputI2C::SignalPlayback(int fd, uint32_t track) {
  if (track == 15) {
    SetLEDOff(i2c_dev0_fd, 15);
    SetLEDOn(i2c_dev1_fd, 15);
  } else {
    uint16_t led = track * LEDS_PER_TRACK;
    if (led >= 15 && led < 30) { led -= 15; }
    if (led >= 30) { led -= 30; }
    SetLEDOff(fd, led);
    SetLEDOn(fd, led + 1);
  }
}

void OutputI2C::SignalMuted(int fd, uint32_t track) {
  if (track == 15) {
    SetLEDOff(i2c_dev0_fd, 15);
    SetLEDBlink(i2c_dev1_fd, 15);
  } else {
    uint16_t led = track * LEDS_PER_TRACK;
    if (led >= 15 && led < 30) { led -= 15; }
    if (led >= 30) { led -= 30; }
    SetLEDOff(fd, led);
    SetLEDBlink(fd, led + 1);
  }
}

void OutputI2C::SignalOff(int fd, uint32_t track) {
  if (track == 15) {
    SetLEDOff(i2c_dev0_fd, 15);
    SetLEDOff(i2c_dev1_fd, 15);
  } else {
    uint16_t led = track * LEDS_PER_TRACK;
    if (led >= 15 && led < 30) { led -= 15; }
    if (led >= 30) { led -= 30; }
    SetLEDOff(fd, led);
    SetLEDOff(fd, led + 1);
  }
}

void OutputI2C::SignalInGroup(int fd, uint32_t track) {
  if (track == 15) {
    SetLEDOn(i2c_dev2_fd, track);
  } else {
    uint16_t led = track * LEDS_PER_TRACK;
    if (led >= 15 && led < 30) { led -= 15; }
    if (led >= 30) { led -= 30; }
    SetLEDOn(fd, led + 2);
  }
}

void OutputI2C::SignalNotInGroup(int fd, uint32_t track) {
  if (track == 15) {
    SetLEDOff(i2c_dev2_fd, track);
  } else {
    uint16_t led = track * LEDS_PER_TRACK;
    // 5 tracks per expander, 15 LEDs per expander except track 15
    if (led >= 15 && led < 30) { led -= 15; }
    if (led >= 30) { led -= 30; }
    SetLEDOff(fd, led + 2);
  }
}

// Make these threads and run and die
// Update to select fd based upon range of track:
// 3 LED per track
// Dev0 0:2 - track 0
// Dev0 3:5 - track 1
// Dev0 6:8 - track 2
// Dev0 9:11 - track 3
// Dev0 12:14 - track 4
// Dev0 15 - track 15 red
//
// Dev1 0:2 - track 5
// Dev1 3:5 - track 6
// Dev1 6:8 - track 7
// Dev1 9:11 - track 8
// Dev1 12:14 - track 9
// Dev1 15 - track 15 green
//
// Dev2 0:2 - track 10
// Dev2 3:5 - track 11
// Dev2 6:8 - track 12
// Dev2 9:11 - track 13
// Dev2 12:14 - track 14
// Dev2 15 - track 15 yellow
//
// groups 0-7 occupy dev3 - maybe

int OutputI2C::TrackToDevFd(uint32_t track) {
  if (track < 5) {
    return i2c_dev0_fd;
  } else if (track >= 5 && track < 10) {
    return i2c_dev1_fd;
  } else if (track >= 10 && track < 15) {
    return i2c_dev2_fd;
  } else {
    return -1;
  }
}
void OutputI2C::SignalTrackRecording(uint32_t track) {
  std::thread t(&OutputI2C::SignalRecord, this, TrackToDevFd(track), track);
  t.detach();
}
// solid green only - playback and repeat
void OutputI2C::SignalTrackPlayback(uint32_t track) {
  std::thread t(&OutputI2C::SignalPlayback, this, TrackToDevFd(track), track);
  t.detach();
}
// blink green only - don't set when switching groups
void OutputI2C::SignalTrackMuted(uint32_t track) {
  std::thread t(&OutputI2C::SignalMuted, this, TrackToDevFd(track), track);
  t.detach();
}
// turn off LEDs - off state or not member of active group
void OutputI2C::SignalTrackOff(uint32_t track) {
  std::thread t(&OutputI2C::SignalOff, this, TrackToDevFd(track), track);
  t.detach();
}
// b - turn off all LEDs first then on bits set in tracks
// tracks in playback - set LED
// record will never be
// tracks in mute
// tracks off
void OutputI2C::SignalTracksInGroupThread(
     uint16_t tracks_in_group,
     uint16_t tracks_in_playback,
     uint16_t tracks_in_mute,
     uint16_t tracks_off) {
  // clear all track LED's other than group
  for (uint32_t bit = 0; bit < 16; bit++) {
    SignalOff(TrackToDevFd(bit), bit);
  }
  // Signal members of group
  for (uint32_t bit = 0; bit < 15; bit++) {
    if (tracks_in_group & (0x1 << bit)) {
      SignalInGroup(TrackToDevFd(bit), bit);
    } else {
      SignalNotInGroup(TrackToDevFd(bit), bit);
    }
  }
  if (tracks_in_group & (0x1 << 15)) {
    SignalInGroup(TrackToDevFd(15), 15);
  } else {
    SignalNotInGroup(TrackToDevFd(15), 15);
  }

  // Signal tracks in playback if members of group
  for (uint32_t bit = 0; bit < 15; bit++) {
    if (tracks_in_playback & (0x1 << bit) &&
	tracks_in_group & (0x1 << bit)) {
      SignalPlayback(TrackToDevFd(bit), bit);
    } 
  }
  if (tracks_in_playback & (0x1 << 15) &&
      tracks_in_group & (0x1 << 15)) {
    SignalPlayback(TrackToDevFd(15), 15);
  }

  // Signal tracks in mute if members of group
  for (uint32_t bit = 0; bit < 15; bit++) {
    if (tracks_in_mute & (0x1 << bit) &&
	tracks_in_group & (0x1 << bit)) {
      SignalMuted(TrackToDevFd(bit), bit);
    } 
  }
  if (tracks_in_mute & (0x1 << 15) &&
      tracks_in_group & (0x1 << 15)) {
    SignalMuted(TrackToDevFd(15), 15);
  }

  // Signal tracks off if members of group
  for (uint32_t bit = 0; bit < 15; bit++) {
    if (tracks_off & (0x1 << bit) &&
	tracks_in_group & (0x1 << bit)) {
      SignalOff(TrackToDevFd(bit), bit);
    } 
  }
  if (tracks_off & (0x1 << 15) &&
      tracks_in_group & (0x1 << 15)) {
    SignalOff(TrackToDevFd(15), 15);
  }

}

// b - turn off all LEDs first then on bits set in tracks
void OutputI2C::SignalTracksInGroup(
     uint16_t tracks_in_group,
     uint16_t tracks_in_playback,
     uint16_t tracks_in_mute,
     uint16_t tracks_off) {
  std::thread t(&OutputI2C::SignalTracksInGroupThread, this,
                tracks_in_group,
		tracks_in_playback,
		tracks_in_mute,
		tracks_off);
  t.detach();
}

