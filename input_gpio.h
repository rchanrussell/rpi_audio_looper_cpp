#ifndef INPUT_GPIO_H
#define INPUT_GPIO_H

#define DOUBLE_DOWN_TIME_S 1
#define SHORT_PULSE_TIME_S 1
#define SHORT_PULSE_TIME_US 500000
#define DOUBLE_SHORT_PULSE_TIME_S 1
#define DOUBLE_SHORT_PULSE_TIME_US 500000
#define LONG_PULSE_TIME_S 1
#define LONG_PULSE_TIME_US 500000
#define DEBOUNCE_TIME_US 20000
#define MAX_EVENT_QUEUE_SIZE 8

enum class InputProcessedEvent {
  kNo = 0,
  kDown,
  kUp,
  kDoubleDown,
  kShortPulse,
  kLongPulse
};

struct ProcessedEvent {
  InputProcessedEvent event;
  int input_number;
  bool is_track;
};

class InputGpio {
  // Variables
  volatile uint8_t processed_event_counter;
  volatile ProcessedEvent processed_event_queue[MAX_EVENT_QUEUE_SIZE];
  InputProcessedEvent last_event;
  uint16_t last_track;
  uint8_t last_group;
  bool last_event_for_track;

  // Methods
  bool ConfigureWiringPiPins();

  void ProcessedEventQueueUpdate();

  void EnqueueInputDownEvent(uint32_t input_number, bool is_track);
  void EnqueueInputUpEvent(uint32_t input_number, bool is_track);
  void GpioIsrProcessor(uint32_t wpi_pin, uint32_t input_number, bool is_track);

  // ISRs
  static void Gpio_Track_IsrHandler(struct WPIWfiStatus wfiStatus, void* userdata);
  static void Gpio_Group_IsrHandler(struct WPIWfiStatus wfiStatus, void* userdata);

  public:
  InputGpio();
  static InputGpio* instance;

  bool InitializeWiringPiGpio();
  bool ProcessAndHandleInputEvents();
  void Reset();

  bool LastEventWasDown();
  bool LastEventWasUp();
  bool LastEventWasDoubleDown();
  bool LastEventWasShortPulse();
  bool LastEventWasLongPulse();
  bool LastEventWasForTrack();
  int GetLastTrack();
  int GetLastGroup();
};

#endif //INPUT_GPIO_H
