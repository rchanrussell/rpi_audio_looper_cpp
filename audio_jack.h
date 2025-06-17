#ifndef AUDIO_JACK_H
#define AUDIO_JACK_H

#include <jack/jack.h>
#include "track_manager.h"
#include "input_gpio.h"

class TrackManager;
class InputGpio;

class AudioJack {

  // Internal only
  static jack_port_t *input_port1;
  static jack_port_t *input_port2;
  static jack_port_t *output_port1;
  static jack_port_t *output_port2;
  static jack_client_t *client;
  // When the buffer is full we need these objects
  typedef struct {
    TrackManager* track_manager_left_;
    TrackManager* track_manager_right_;
    InputGpio* gpio_;
    bool enabled;
  } ProcessVars;
  ProcessVars pv_;

  static void SignalHandler(int sig);
  static void JackShutdown(void *arg);
  static int Process(jack_nframes_t nframes, void *arg);

  public:
  AudioJack();
  ~AudioJack();

  int Init(int argc, char *argv[]);
  void SetTrackManagerPtr(TrackManager* tm_left, TrackManager* tm_right);
  void SetInputGpioPtr(InputGpio* gpio);

  void EnableJackAudioProcessing();
};

#endif // AUDIO_JACK_H
