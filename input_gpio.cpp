#include <iostream>
#include <string>
#include <vector>
#include <wiringPi.h>
#include <unistd.h>
#include <sys/time.h>
#include <chrono>
#include <thread>
#include "input_gpio.h"
#include "rpi_io_to_app_map.h"

InputGpio* InputGpio::instance = nullptr;

static void InputProcessedEventToText(InputProcessedEvent event) {
    if (event == InputProcessedEvent::kNo) {
      std::cout << "No Event";
    }
    if (event == InputProcessedEvent::kDown) {
      std::cout << "Down Event";
    }
    if (event == InputProcessedEvent::kUp) {
      std::cout << "Up Event";
    }
    if (event == InputProcessedEvent::kDoubleDown) {
      std::cout << "DoubleDown Event";
    }
    if (event == InputProcessedEvent::kShortPulse) {
      std::cout << "ShortPulse Event";
    }
    if (event == InputProcessedEvent::kLongPulse) {
      std::cout << "LongPulse Event";
    }
}

// Static Methods
void InputGpio::EnqueueInputDownEvent(uint32_t input_number) {
  processed_event_queue[++processed_event_counter].event = InputProcessedEvent::kDown;
  processed_event_queue[processed_event_counter].wpi_pin = input_number;
}

void InputGpio::EnqueueInputUpEvent(uint32_t input_number) {
  processed_event_queue[++processed_event_counter].event = InputProcessedEvent::kUp;
  processed_event_queue[processed_event_counter].wpi_pin = input_number;
}

void InputGpio::GpioIsrProcessor(uint32_t input_number) {
  static int last_read = -1;
  int current_read = digitalRead(input_number);
  if (last_read == current_read) {
    return;
  }
  // Debounce
  struct timeval current;
  struct timeval diff;
  static struct timeval last_gpio = (struct timeval){0};
  gettimeofday(&current, NULL);
  timersub(&current, &last_gpio, &diff);
  last_gpio = current;
  if (diff.tv_usec < DEBOUNCE_TIME_US) {
    return;
  }

  if (current_read == LOW) {
    InputGpio::EnqueueInputDownEvent(input_number);
  } else {
    InputGpio::EnqueueInputUpEvent(input_number);
  }
  last_read = current_read;
}

// Sadly, wiringPi doesn't have a means of notifying the user which GPIO has encountered
// an interrupt. So we have to create unique functions for each GPIO, instead of one
// generic handler
// Tracks
void InputGpio::Gpio_0_IsrHandler() {
  if (instance) {
    instance->GpioIsrProcessor(track_to_pin.at(0).wiring_pi);
  }
}

void InputGpio::Gpio_1_IsrHandler() {
  if (instance) {
    instance->GpioIsrProcessor(track_to_pin.at(1).wiring_pi);
  }
}

void InputGpio::Gpio_2_IsrHandler() {
  if (instance) {
    instance->GpioIsrProcessor(track_to_pin.at(2).wiring_pi);
  }
}

void InputGpio::Gpio_3_IsrHandler() {
  if (instance) {
    instance->GpioIsrProcessor(track_to_pin.at(3).wiring_pi);
  }
}

void InputGpio::Gpio_4_IsrHandler() {
  if (instance) {
    instance->GpioIsrProcessor(track_to_pin.at(4).wiring_pi);
  }
}

void InputGpio::Gpio_5_IsrHandler() {
  if (instance) {
    instance->GpioIsrProcessor(track_to_pin.at(5).wiring_pi);
  }
}

void InputGpio::Gpio_6_IsrHandler() {
  if (instance) {
    instance->GpioIsrProcessor(track_to_pin.at(6).wiring_pi);
  }
}

void InputGpio::Gpio_7_IsrHandler() {
  if (instance) {
    instance->GpioIsrProcessor(track_to_pin.at(7).wiring_pi);
  }
}

void InputGpio::Gpio_8_IsrHandler() {
  if (instance) {
    instance->GpioIsrProcessor(track_to_pin.at(8).wiring_pi);
  }
}

void InputGpio::Gpio_9_IsrHandler() {
  if (instance) {
    instance->GpioIsrProcessor(track_to_pin.at(9).wiring_pi);
  }
}

void InputGpio::Gpio_10_IsrHandler() {
  if (instance) {
    instance->GpioIsrProcessor(track_to_pin.at(10).wiring_pi);
  }
}

void InputGpio::Gpio_11_IsrHandler() {
  if (instance) {
    instance->GpioIsrProcessor(track_to_pin.at(11).wiring_pi);
  }
}

void InputGpio::Gpio_12_IsrHandler() {
  if (instance) {
    instance->GpioIsrProcessor(track_to_pin.at(12).wiring_pi);
  }
}

void InputGpio::Gpio_13_IsrHandler() {
  if (instance) {
    instance->GpioIsrProcessor(track_to_pin.at(13).wiring_pi);
  }
}

void InputGpio::Gpio_14_IsrHandler() {
  if (instance) {
    instance->GpioIsrProcessor(track_to_pin.at(14).wiring_pi);
  }
}

void InputGpio::Gpio_15_IsrHandler() {
  if (instance) {
    instance->GpioIsrProcessor(track_to_pin.at(15).wiring_pi);
  }
}

// Groups
void InputGpio::Gpio_16_IsrHandler() {
  if (instance) {
    instance->GpioIsrProcessor(group_to_pin.at(0).wiring_pi);
  }
}

void InputGpio::Gpio_17_IsrHandler() {
  if (instance) {
    instance->GpioIsrProcessor(group_to_pin.at(1).wiring_pi);
  }
}

void InputGpio::Gpio_18_IsrHandler() {
  if (instance) {
    instance->GpioIsrProcessor(group_to_pin.at(2).wiring_pi);
  }
}

void InputGpio::Gpio_19_IsrHandler() {
  if (instance) {
    instance->GpioIsrProcessor(group_to_pin.at(3).wiring_pi);
  }
}

void InputGpio::Gpio_20_IsrHandler() {
  if (instance) {
    instance->GpioIsrProcessor(group_to_pin.at(4).wiring_pi);
  }
}

void InputGpio::Gpio_21_IsrHandler() {
  if (instance) {
    instance->GpioIsrProcessor(group_to_pin.at(5).wiring_pi);
  }
}

void InputGpio::Gpio_22_IsrHandler() {
  if (instance) {
    instance->GpioIsrProcessor(group_to_pin.at(6).wiring_pi);
  }
}

void InputGpio::Gpio_23_IsrHandler() {
  if (instance) {
    instance->GpioIsrProcessor(group_to_pin.at(7).wiring_pi);
  }
}


InputGpio::InputGpio() {
  instance = this;
  Reset();
}

void InputGpio::Reset() {
  processed_event_counter = 0;
  for (auto &e : processed_event_queue) {
    e.event = InputProcessedEvent::kNo;
    e.wpi_pin = 0;
  }
  last_event = InputProcessedEvent::kNo;
  last_group = -1;
  last_track = -1;
  last_event_for_track = false;
}

// Events of concern for the state machine are
// Down, Double Down (2 downs within 1s), and Long Pulse (up 1s after down)
void InputGpio::ProcessedEventQueueUpdate() {
  struct timeval current;
  struct timeval diff;
  static struct timeval last_down = (struct timeval){0};

  gettimeofday(&current, NULL);

  if (processed_event_queue[processed_event_counter].event == InputProcessedEvent::kDown) {
    if (last_down.tv_sec != 0 && last_down.tv_usec != 0) {
      // check if double-down -- two downs within limit, insert
      // This will cause two fast transitions
      timersub(&current, &last_down, &diff);
      if (diff.tv_sec < DOUBLE_DOWN_TIME_S) {
        processed_event_queue[processed_event_counter].event = InputProcessedEvent::kDoubleDown;
      }
    }
    last_down = current;
    return;
  }
  // With up events we want to replace the UP with a Pulse event
  // Currently, only the Long Pulse is used
  // The downs should trigger the main action with the exception of a long
  // awaited up --> a long pulse
  if (processed_event_queue[processed_event_counter].event == InputProcessedEvent::kUp) {
    // Received Up - check if PB was held down for a long pulse
    timersub(&current, &last_down, &diff);
    if (diff.tv_sec > LONG_PULSE_TIME_S || ( diff.tv_sec == LONG_PULSE_TIME_S && diff.tv_usec > LONG_PULSE_TIME_US)) {
      processed_event_queue[processed_event_counter].event = InputProcessedEvent::kLongPulse;
    } else {
      processed_event_queue[processed_event_counter].event = InputProcessedEvent::kShortPulse;
    }
  }
}

bool InputGpio::ConfigureWiringPiPins() {
  for (int i = 0; i < 16; i++) {
    pinMode(track_to_pin.at(i).wiring_pi, INPUT);
    pullUpDnControl(track_to_pin.at(i).wiring_pi, PUD_UP);
  }
  for (int i = 0; i < 8; i++) {
    pinMode(group_to_pin.at(i).wiring_pi, INPUT);
    pullUpDnControl(group_to_pin.at(i).wiring_pi, PUD_UP);
  }
  return true;
}

bool InputGpio::AssignWiringPiISRs() {
  /* Tracks - index of track_to_pin starts at 0 and represents Track 0
   * index 1 represets Track 1, etc..
   */
  if (wiringPiISR(track_to_pin.at(0).wiring_pi, INT_EDGE_BOTH, &Gpio_0_IsrHandler) < 0) {
    std::cout << "Unable to setup ISR PB 0" << std::endl;
    return false;
  }

  if (wiringPiISR(track_to_pin.at(1).wiring_pi, INT_EDGE_BOTH, &Gpio_1_IsrHandler) < 0) {
    std::cout << "Unable to setup ISR PB 1" << std::endl;
    return false;
  }

  if (wiringPiISR(track_to_pin.at(2).wiring_pi, INT_EDGE_BOTH, &Gpio_2_IsrHandler) < 0) {
    std::cout << "Unable to setup ISR PB 2" << std::endl;
    return false;
  }

  if (wiringPiISR(track_to_pin.at(3).wiring_pi, INT_EDGE_BOTH, &Gpio_3_IsrHandler) < 0) {
    std::cout << "Unable to setup ISR PB 3" << std::endl;
    return false;
  }

  if (wiringPiISR(track_to_pin.at(4).wiring_pi, INT_EDGE_BOTH, &Gpio_4_IsrHandler) < 0) {
    std::cout << "Unable to setup ISR PB 4" << std::endl;
    return false;
  }

  if (wiringPiISR(track_to_pin.at(5).wiring_pi, INT_EDGE_BOTH, &Gpio_5_IsrHandler) < 0) {
    std::cout << "Unable to setup ISR PB 5" << std::endl;
    return false;
  }

  if (wiringPiISR(track_to_pin.at(6).wiring_pi, INT_EDGE_BOTH, &Gpio_6_IsrHandler) < 0) {
    std::cout << "Unable to setup ISR PB 6" << std::endl;
    return false;
  }

  if (wiringPiISR(track_to_pin.at(7).wiring_pi, INT_EDGE_BOTH, &Gpio_7_IsrHandler) < 0) {
    std::cout << "Unable to setup ISR PB 7" << std::endl;
    return false;
  }

  if (wiringPiISR(track_to_pin.at(8).wiring_pi, INT_EDGE_BOTH, &Gpio_8_IsrHandler) < 0) {
    std::cout << "Unable to setup ISR PB 8" << std::endl;
    return false;
  }

  if (wiringPiISR(track_to_pin.at(9).wiring_pi, INT_EDGE_BOTH, &Gpio_9_IsrHandler) < 0) {
    std::cout << "Unable to setup ISR PB 9" << std::endl;
    return false;
  }

  if (wiringPiISR(track_to_pin.at(10).wiring_pi, INT_EDGE_BOTH, &Gpio_10_IsrHandler) < 0) {
    std::cout << "Unable to setup ISR PB 10" << std::endl;
    return false;
  }

  if (wiringPiISR(track_to_pin.at(11).wiring_pi, INT_EDGE_BOTH, &Gpio_11_IsrHandler) < 0) {
    std::cout << "Unable to setup ISR PB 11" << std::endl;
    return false;
  }

  if (wiringPiISR(track_to_pin.at(12).wiring_pi, INT_EDGE_BOTH, &Gpio_12_IsrHandler) < 0) {
    std::cout << "Unable to setup ISR PB 12" << std::endl;
    return false;
  }

  if (wiringPiISR(track_to_pin.at(13).wiring_pi, INT_EDGE_BOTH, &Gpio_13_IsrHandler) < 0) {
    std::cout << "Unable to setup ISR PB 13" << std::endl;
    return false;
  }

  if (wiringPiISR(track_to_pin.at(14).wiring_pi, INT_EDGE_BOTH, &Gpio_14_IsrHandler) < 0) {
    std::cout << "Unable to setup ISR PB 14" << std::endl;
    return false;
  }

  if (wiringPiISR(track_to_pin.at(15).wiring_pi, INT_EDGE_BOTH, &Gpio_15_IsrHandler) < 0) {
    std::cout << "Unable to setup ISR PB 15" << std::endl;
    return false;
  }


  /* Groups - index of group_to_pin starts at 0 and represets Group 0
   * index 1 represents Group 1, etc..
   */
  if (wiringPiISR(group_to_pin.at(0).wiring_pi, INT_EDGE_BOTH, &Gpio_16_IsrHandler) < 0) {
    std::cout << "Unable to setup ISR PB 16" << std::endl;
    return false;
  }
  if (wiringPiISR(group_to_pin.at(1).wiring_pi, INT_EDGE_BOTH, &Gpio_17_IsrHandler) < 0) {
    std::cout << "Unable to setup ISR PB 17" << std::endl;
    return false;
  }

  if (wiringPiISR(group_to_pin.at(2).wiring_pi, INT_EDGE_BOTH, &Gpio_18_IsrHandler) < 0) {
    std::cout << "Unable to setup ISR PB 18" << std::endl;
    return false;
  }
  if (wiringPiISR(group_to_pin.at(3).wiring_pi, INT_EDGE_BOTH, &Gpio_19_IsrHandler) < 0) {
    std::cout << "Unable to setup ISR PB 19" << std::endl;
    return false;
  }
  if (wiringPiISR(group_to_pin.at(4).wiring_pi, INT_EDGE_BOTH, &Gpio_20_IsrHandler) < 0) {
    std::cout << "Unable to setup ISR PB 20" << std::endl;
    return false;
  }
  if (wiringPiISR(group_to_pin.at(5).wiring_pi, INT_EDGE_BOTH, &Gpio_21_IsrHandler) < 0) {
    std::cout << "Unable to setup ISR PB 21" << std::endl;
    return false;
  }

  if (wiringPiISR(group_to_pin.at(6).wiring_pi, INT_EDGE_BOTH, &Gpio_22_IsrHandler) < 0) {
    std::cout << "Unable to setup ISR PB 22" << std::endl;
    return false;
  }
  if (wiringPiISR(group_to_pin.at(7).wiring_pi, INT_EDGE_BOTH, &Gpio_23_IsrHandler) < 0) {
    std::cout << "Unable to setup ISR PB 23" << std::endl;
    return false;
  }

  return true;
}

bool InputGpio::InitializeWiringPiGpio() {
  wiringPiSetup();
  if (!ConfigureWiringPiPins()) {
    std::cout << "Error intializing WiringPi - Pin Configuration" << std::endl;
    return false;
  }
  if (!AssignWiringPiISRs()) {
    std::cout << "Error intializing WiringPi - ISR Assignment" << std::endl;
    return false;
  }
  return true;
}

bool InputGpio::ProcessAndHandleInputEvents() {
  if (processed_event_counter == 0) {return false;}

  if (processed_event_counter > 1) {
    std::cout << "***** MORE THAN ONE EVENT: " << processed_event_counter << std::endl;
    for (int i = processed_event_counter; i > 0; i--) {
      std::cout << "    " << processed_event_queue[i].wpi_pin;
      std::cout << ", ";
      InputProcessedEventToText(processed_event_queue[i].event);
      std::cout << std::endl;
    }
  }

  while (processed_event_counter > 0) {
    // process events - updating as needed
    ProcessedEventQueueUpdate();
    struct ProcessedEvent e;
    e.wpi_pin = processed_event_queue[processed_event_counter].wpi_pin;
    e.event = processed_event_queue[processed_event_counter].event;
    uint32_t track = 0;
    uint32_t group = 0;

    //TODO When integrating, replace 16 and 8 with defines from headers for each
    for (track = 0; track < 16; track++) {
      if (e.wpi_pin == track_to_pin.at(track).wiring_pi) {
	break;
      }
    }
    if (track < 16) {
#ifdef DTEST_GPIO_VERBOSE
      std::cout << "ProcessAndHandleInputEvents() - track: " << track << " had event: ";
      InputProcessedEventToText(e.event);
      std::cout << std::endl;
#endif
      last_event_for_track = true;
      last_track = track;
    }
    if (track == 16) {
      for (group = 0; group < 8; group++) {
        if (e.wpi_pin == group_to_pin.at(group).wiring_pi) {
          break;
        }
      }
      if (group < 8) {
  #ifdef DTEST_GPIO_VERBOSE
        std::cout << "ProcessAndHandleInputEvents() - group: " << group << " had event: ";
        InputProcessedEventToText(e.event);
        std::cout << std::endl;
  #endif
        last_event_for_track = false;
        last_group = group;
      }
    }
    last_event = e.event;
    processed_event_counter--;
  }
  return true;
}

bool InputGpio::LastEventWasDown() {
  return InputProcessedEvent::kDown == last_event;
}

bool InputGpio::LastEventWasUp() {
  return InputProcessedEvent::kUp == last_event;
}

bool InputGpio::LastEventWasDoubleDown() {
  return InputProcessedEvent::kDoubleDown == last_event;
}

bool InputGpio::LastEventWasShortPulse() {
  return InputProcessedEvent::kShortPulse == last_event;
}

bool InputGpio::LastEventWasLongPulse() {
  return InputProcessedEvent::kLongPulse == last_event;
}

bool InputGpio::LastEventWasForTrack() {
  return last_event_for_track;
}

int InputGpio::GetLastTrack() {
  return last_track;
}

int InputGpio::GetLastGroup() {
  return last_group;
}
