#ifndef RPI_IO_TO_APP_MAP_H
#define RPI_IO_TO_APP_MAP_H

#include <vector>

struct WiringPiToGpioHeaderPin {
  int wiring_pi;
  int header_pin;
  unsigned int bcm_pin;
  unsigned int input_number;
};

// Set this in order - first entry is track 0
// second entry is track 1 etc..
// You can reorder the groupings but...
// DO NOT separate the groupings - don't mix 0 and 13, this isn't
// how wiringPi works and you'll encounter horrible bugs!
// Keep this only for track inputs!
std::vector<struct WiringPiToGpioHeaderPin> track_to_pin = {
  {13, 21, 9, 0},  {14, 23, 11, 1}, {30, 27, 0, 2},  {21, 29, 5, 3},
  {25, 37, 26, 4}, {24, 35, 19, 5}, {23, 33, 13, 6}, {22, 31, 6, 7},
  {0, 11, 17, 8},  {2, 13, 27, 9},  {3, 15, 22, 10}, {12, 19, 10, 11},
  {16, 10, 15, 12},{1, 12, 18, 13},  {4, 16, 23, 14},{5, 18, 24, 15}
};

// Set this in order - first entry is group 0
// second entry is group 1 etc..
std::vector<struct WiringPiToGpioHeaderPin> group_to_pin = {
  {29, 40, 21, 0}, {28, 38, 20, 1}, {27, 36, 16, 2}, {26, 32, 12, 3},
  {6, 22, 25, 4},  {10, 24, 8, 5},  {11, 26, 7, 6},  {31, 28, 1, 7}
};

#endif //RPI_IO_TO_APP_MAP_H
