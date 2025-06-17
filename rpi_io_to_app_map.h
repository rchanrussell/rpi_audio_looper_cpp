#ifndef RPI_IO_TO_APP_MAP_H
#define RPI_IO_TO_APP_MAP_H

#include <vector>

struct WiringPiToGpioHeaderPin {
  int wiring_pi;
  int header_pin;
};

// Set this in order - first entry is track 0
// second entry is track 1 etc..
// You can reorder the groupings but...
// DO NOT separate the groupings - don't mix 0 and 13, this isn't
// how wiringPi works and you'll encounter horrible bugs!
// Keep this only for track inputs!
std::vector<struct WiringPiToGpioHeaderPin> track_to_pin = {
  /* Odd Header Pins */
  {0, 11},  {2, 13},  {3, 15},  {12, 19},
  {13, 21}, {14, 23}, {30, 27}, {21, 29},
  {22, 31}, {23, 33}, {24, 35}, {25, 37},
  /* Even Header Pins */
  {15, 8},  {16, 10}, {1, 12},  {4, 16}
};

// Set this in order - first entry is group 0
// second entry is group 1 etc..
std::vector<struct WiringPiToGpioHeaderPin> group_to_pin = {
  {5, 18},  {6, 22},  {10, 24}, {11, 26},
  {31, 28}, {26, 32}, {27, 36}, {28, 38},
};

// Simple map for LED access using SDA/SCL
// WiringPi uses I2C1, header pins 3 and 5
// No need for defines, but do not connect with them if
// using I2C and wiringPi as this app does, for LED output

std::vector<struct WiringPiToGpioHeaderPin> wiringpi_all_gpio_to_pin = {
  /* Odd Header Pins */
  {8, 3},   {9, 5},   {7, 7},   {0, 11},
  {2, 13},  {3, 15},  {12, 19}, {13, 21},
  {14, 23}, {30, 27}, {21, 29}, {22, 31},
  {23, 33}, {24, 35}, {25, 37},
  /* Even Header Pins */
  {15, 8},  {16, 10}, {1, 12},  {4, 16},  {5, 18},  {6, 22},
  {10, 24}, {11, 26}, {31, 28}, {26, 32}, {27, 36}, {28, 38},
  {29, 40}
};

#endif //RPI_IO_TO_APP_MAP_H
