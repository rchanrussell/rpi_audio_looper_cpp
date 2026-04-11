#include <iostream>
#include <string>
#include <vector>
#include <wiringPi.h>
#include <unistd.h>
#include <sys/time.h>
#include <chrono>
#include <thread>
#include "util.h"
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
void InputGpio::EnqueueInputDownEvent(uint32_t input_number, bool is_track) {
  processed_event_queue[++processed_event_counter].event = InputProcessedEvent::kDown;
  processed_event_queue[processed_event_counter].input_number = input_number;
  processed_event_queue[processed_event_counter].is_track = is_track;
}

void InputGpio::EnqueueInputUpEvent(uint32_t input_number, bool is_track) {
  processed_event_queue[++processed_event_counter].event = InputProcessedEvent::kUp;
  processed_event_queue[processed_event_counter].input_number = input_number;
  processed_event_queue[processed_event_counter].is_track = is_track;
}

void InputGpio::GpioIsrProcessor(uint32_t wpi_pin, uint32_t input_number, bool is_track) {
#ifdef DTEST_GPIO_VERBOSE
  std::cout << "GIP:" << input_number << ", T?:" << is_track << std::endl;
#endif
  static int last_read = -1;
  int current_read = digitalRead(wpi_pin);
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
    InputGpio::EnqueueInputDownEvent(input_number, is_track);
  } else {
    InputGpio::EnqueueInputUpEvent(input_number, is_track);
  }
  last_read = current_read;
}

void InputGpio::Gpio_Track_IsrHandler(struct WPIWfiStatus wfiStatus, void* userdata) {
#ifdef DTEST_GPIO_VERBOSE
  std::cout << "TrackISR: OK:" << wfiStatus.statusOK << ", pinBCM:" << wfiStatus.pinBCM;
  std::cout << ", Edge:" << wfiStatus.edge << std::endl;
#endif
  for ( auto p : track_to_pin) {
    if (p.bcm_pin == wfiStatus.pinBCM) {
      if (instance) {
        instance->GpioIsrProcessor(p.wiring_pi, p.input_number, true);
	return;
      }
    }
  }
}

void InputGpio::Gpio_Group_IsrHandler(struct WPIWfiStatus wfiStatus, void* userdata) {
#ifdef DTEST_GPIO_VERBOSE
  std::cout << "GroupISR: OK:" << wfiStatus.statusOK << ", pinBCM:" << wfiStatus.pinBCM;
  std::cout << ", Edge:" << wfiStatus.edge << std::endl;
#endif
  for (auto p : group_to_pin) {
    if (p.bcm_pin == wfiStatus.pinBCM) {
      if (instance) {
        instance->GpioIsrProcessor(p.wiring_pi, p.input_number, false);
	return;
      }
    }
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
    e.input_number = 0;
  }
  last_event = InputProcessedEvent::kNo;
  last_group = MAX_GROUP_COUNT;
  last_track = MAX_TRACK_COUNT;
  last_event_for_track = false;
}

// Events of concern for the state machine are
// Down, Double Down (2 downs within 1s), and Long Pulse (up 1s after down)
void InputGpio::ProcessedEventQueueUpdate() {
  struct timeval current;
  struct timeval diff;
  static struct timeval last_down = (struct timeval){0};
  static int last_pin = 0;
  int current_pin = processed_event_queue[processed_event_counter].input_number;
  gettimeofday(&current, NULL);

  if (processed_event_queue[processed_event_counter].event == InputProcessedEvent::kDown) {
    if (last_down.tv_sec != 0 && last_down.tv_usec != 0) {
      // check if double-down -- two downs within limit, insert
      // This will cause two fast transitions
      timersub(&current, &last_down, &diff);
      if (current_pin == last_pin && diff.tv_sec < DOUBLE_DOWN_TIME_S) {
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
    if (current_pin == last_pin && (diff.tv_sec > LONG_PULSE_TIME_S || 
       (diff.tv_sec == LONG_PULSE_TIME_S && diff.tv_usec > LONG_PULSE_TIME_US))) {
      processed_event_queue[processed_event_counter].event = InputProcessedEvent::kLongPulse;
    } else {
      processed_event_queue[processed_event_counter].event = InputProcessedEvent::kShortPulse;
    }
  }
  last_pin = processed_event_queue[processed_event_counter].input_number; 
}

bool InputGpio::ConfigureWiringPiPins() {
  for (long unsigned int i = 0; i < track_to_pin.size(); i++) {
    pinMode(track_to_pin.at(i).wiring_pi, INPUT);
    pullUpDnControl(track_to_pin.at(i).wiring_pi, PUD_UP);
    wiringPiISR2(track_to_pin.at(i).wiring_pi, INT_EDGE_BOTH, &Gpio_Track_IsrHandler, 1000, NULL); 
  }
  for (long unsigned int i = 0; i < group_to_pin.size(); i++) {
    pinMode(group_to_pin.at(i).wiring_pi, INPUT);
    pullUpDnControl(group_to_pin.at(i).wiring_pi, PUD_UP);
    wiringPiISR2(group_to_pin.at(i).wiring_pi, INT_EDGE_BOTH, &Gpio_Group_IsrHandler, 1000, NULL); 
  }
  return true;
}

bool InputGpio::InitializeWiringPiGpio() {
  wiringPiSetup();
  if (!ConfigureWiringPiPins()) {
    std::cout << "Error intializing WiringPi - Pin Configuration" << std::endl;
    return false;
  }
  return true;
}

bool InputGpio::ProcessAndHandleInputEvents() {
  if (processed_event_counter == 0) {return false;}

  if (processed_event_counter > 1) {
    std::cout << "***** MORE THAN ONE EVENT: " << processed_event_counter << std::endl;
    for (int i = processed_event_counter; i > 0; i--) {
      std::cout << "    " << processed_event_queue[i].input_number;
      std::cout << ", ";
      InputProcessedEventToText(processed_event_queue[i].event);
      std::cout << std::endl;
    }
  }

  while (processed_event_counter > 0) {
    // process events - updating as needed
    ProcessedEventQueueUpdate();
    struct ProcessedEvent e;
    e.input_number = processed_event_queue[processed_event_counter].input_number;
    e.event = processed_event_queue[processed_event_counter].event;
    e.is_track = processed_event_queue[processed_event_counter].is_track;

    if (e.is_track) {
#ifdef DTEST_GPIO_VERBOSE
      std::cout << "ProcessAndHandleInputEvents() - track: " << e.input_number << " had event: ";
      InputProcessedEventToText(e.event);
      std::cout << std::endl;
#endif
      last_event_for_track = true;
      last_track = e.input_number;
    } else {
  #ifdef DTEST_GPIO_VERBOSE
      std::cout << "ProcessAndHandleInputEvents() - group: " << e.input_number << " had event: ";
      InputProcessedEventToText(e.event);
      std::cout << std::endl;
  #endif
      last_event_for_track = false;
      last_group = e.input_number;
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
