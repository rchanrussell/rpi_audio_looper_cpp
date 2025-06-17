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
  int wpi_pin;
};

class InputGpio {
  // Variables
  volatile uint8_t processed_event_counter;
  volatile ProcessedEvent processed_event_queue[MAX_EVENT_QUEUE_SIZE];
  InputProcessedEvent last_event;
  int last_track;
  int last_group;
  bool last_event_for_track;

  // Methods
  bool ConfigureWiringPiPins();
  bool AssignWiringPiISRs();

  void ProcessedEventQueueUpdate();

  void EnqueueInputDownEvent(uint32_t input_number);
  void EnqueueInputUpEvent(uint32_t input_number);
  void GpioIsrProcessor(uint32_t input_number);

  // ISRs
  // Tracks
  static void Gpio_0_IsrHandler();
  static void Gpio_1_IsrHandler();
  static void Gpio_2_IsrHandler();
  static void Gpio_3_IsrHandler();
  static void Gpio_4_IsrHandler();
  static void Gpio_5_IsrHandler();
  static void Gpio_6_IsrHandler();
  static void Gpio_7_IsrHandler();
  static void Gpio_8_IsrHandler();
  static void Gpio_9_IsrHandler();
  static void Gpio_10_IsrHandler();
  static void Gpio_11_IsrHandler();
  static void Gpio_12_IsrHandler();
  static void Gpio_13_IsrHandler();
  static void Gpio_14_IsrHandler();
  static void Gpio_15_IsrHandler();
  // Groups
  static void Gpio_16_IsrHandler();
  static void Gpio_17_IsrHandler();
  static void Gpio_18_IsrHandler();
  static void Gpio_19_IsrHandler();
  static void Gpio_20_IsrHandler();
  static void Gpio_21_IsrHandler();
  static void Gpio_22_IsrHandler();
  static void Gpio_23_IsrHandler();

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
