#include <iostream>
#include <string>
#include <unistd.h>
#include <string.h>
#include <sys/time.h>
#include <chrono>
#include <thread>
#include <math.h>
#include <signal.h>
#include <jack/jack.h>
#include "audio_jack.h"
#include "track_manager.h"
#include "input_gpio.h"

// Deal with static variable requirements
jack_port_t* AudioJack::input_port1 = nullptr;
jack_port_t* AudioJack::input_port2 = nullptr;
jack_port_t* AudioJack::output_port1 = nullptr;
jack_port_t* AudioJack::output_port2 = nullptr;
jack_client_t* AudioJack::client = nullptr;


AudioJack::AudioJack() {
  pv_.enabled = false;
}

AudioJack::~AudioJack() {
  pv_.track_manager_left_ = nullptr;
  pv_.track_manager_right_ = nullptr;
  pv_.gpio_ = nullptr;
}

void AudioJack::SignalHandler(int sig) {
  jack_client_close(client);
  fprintf(stderr, "signal received, exiting ...\n");
  exit(0);
}

void AudioJack::SetTrackManagerPtr(TrackManager* tm_left, TrackManager* tm_right) {
  pv_.track_manager_left_ = tm_left;
  pv_.track_manager_right_ = tm_right;
}

void AudioJack::SetInputGpioPtr(InputGpio* gpio) {
  pv_.gpio_ = gpio;
}

int AudioJack::Process(jack_nframes_t nframes, void *arg) {
  jack_default_audio_sample_t *in1, *in2, *out1, *out2;
  ProcessVars *pv = (ProcessVars*)arg;

  in1 = (jack_default_audio_sample_t*)jack_port_get_buffer (input_port1, nframes);
  in2 = (jack_default_audio_sample_t*)jack_port_get_buffer (input_port2, nframes);

  out1 = (jack_default_audio_sample_t*)jack_port_get_buffer (output_port1, nframes);
  out2 = (jack_default_audio_sample_t*)jack_port_get_buffer (output_port2, nframes);

  if (!pv->enabled) { return 0;}
  if (pv->gpio_ == nullptr) {
#ifdef JACK_VERBOSE
    std::cout << "InputGpio Ptr is null!" << std::endl;
#endif
    return 0;
  }
  if (pv->gpio_->GetLastTrack() >= MAX_TRACK_COUNT) { return 0; }

  if (pv->track_manager_left_ == nullptr) {
#ifdef JACK_VERBOSE
    std::cout << "TrackManagerPtr Left is null!" << std::endl;
#endif
    return 0;
  } else {
    if (pv->track_manager_left_->GetTracksOff() == 0xFFFF) { return 0; }
    pv->track_manager_left_->CopyToInputBuffer(in1, SAMPLES_PER_BLOCK);
    pv->track_manager_left_->StateProcess(pv->gpio_->GetLastTrack()); // copies buffer to track, performs mixdown and updates indicies
    pv->track_manager_left_->CopyMixdownToBuffer(out1, nframes);
    if (pv->track_manager_right_ == nullptr) {
      pv->track_manager_left_->CopyMixdownToBuffer(out2, nframes);
    }
  }
  if (pv->track_manager_right_ == nullptr) {
#ifdef JACK_VERBOSE
    std::cout << "TrackManagerPtr Right is null!" << std::endl;
#endif
    return 0;
  } else {
    if (pv->track_manager_right_->GetTracksOff() == 0xFFFF) { return 0; } 
    pv->track_manager_right_->CopyToInputBuffer(in2, SAMPLES_PER_BLOCK);
    pv->track_manager_right_->StateProcess(pv->gpio_->GetLastTrack()); // copies buffer to track, performs mixdown and updates indicies
    pv->track_manager_right_->CopyMixdownToBuffer(out2, nframes);
  }

  return 0;      
}


void AudioJack::JackShutdown(void *arg) {
  exit (1);
}

void AudioJack::EnableJackAudioProcessing() {
  pv_.enabled = true;
}

// This is taken almost verbatim from simple_client.c
int AudioJack::Init(int argc, char *argv[]) {
  const char **ports;
  const char *client_name;
  const char *server_name = NULL;
  jack_options_t options = JackNullOption;
  jack_status_t status;

  if (argc >= 2) {  	/* client name specified? */
    client_name = argv[1];
    if (argc >= 3) {	/* server name specified? */
    	server_name = argv[2];
    	int my_option = JackNullOption | JackServerName;
    	options = (jack_options_t)my_option;
    }
  } else {  		/* use basename of argv[0] */
    client_name = strrchr(argv[0], '/');
    if (client_name == 0) {
    	client_name = argv[0];
    } else {
    	client_name++;
    }
  }

  /* open a client connection to the JACK server */

  client = jack_client_open (client_name, options, &status, server_name);
  if (client == NULL) {
    fprintf (stderr, "jack_client_open() failed, "
    	 "status = 0x%2.0x\n", status);
    if (status & JackServerFailed) {
    	fprintf (stderr, "Unable to connect to JACK server\n");
    }
    exit (1);
  }
  if (status & JackServerStarted) {
    fprintf (stderr, "JACK server started\n");
  }
  if (status & JackNameNotUnique) {
    client_name = jack_get_client_name(client);
    fprintf (stderr, "unique name `%s' assigned\n", client_name);
  }

  /* tell the JACK server to call `process()' whenever
     there is work to be done.
  */

  jack_set_process_callback (client, Process, &pv_);

  /* tell the JACK server to call `jack_shutdown()' if
     it ever shuts down, either entirely, or if it
     just decides to stop calling us.
  */

  jack_on_shutdown (client, JackShutdown, 0);

  /* create two ports */
  input_port1 = jack_port_register (client, "input1",
    			  JACK_DEFAULT_AUDIO_TYPE,
    			  JackPortIsInput, 0);

  input_port2 = jack_port_register (client, "input2",
    			  JACK_DEFAULT_AUDIO_TYPE,
    			  JackPortIsInput, 0);

  if ((input_port1 == NULL) || (input_port2 == NULL)) {
    fprintf(stderr, "no more JACK ports available\n");
    exit (1);
  }

  output_port1 = jack_port_register (client, "output1",
    			  JACK_DEFAULT_AUDIO_TYPE,
    			  JackPortIsOutput, 0);

  output_port2 = jack_port_register (client, "output2",
    			  JACK_DEFAULT_AUDIO_TYPE,
    			  JackPortIsOutput, 0);

  if ((output_port1 == NULL) || (output_port2 == NULL)) {
    fprintf(stderr, "no more JACK ports available\n");
    exit (1);
  }

  /* Tell the JACK server that we are ready to roll.  Our
   * process() callback will start running now. */

  if (jack_activate (client)) {
    fprintf (stderr, "cannot activate client");
    exit (1);
  }

  /* Connect the ports.  You can't do this before the client is
   * activated, because we can't make connections to clients
   * that aren't running.  Note the confusing (but necessary)
   * orientation of the driver backend ports: playback ports are
   * "input" to the backend, and capture ports are "output" from
   * it.
   */

  // IsOutput means data can be read from the port - we read from it
  ports = jack_get_ports (client, NULL, NULL, JackPortIsPhysical|JackPortIsOutput);
 
  if (ports == NULL) {
    fprintf(stderr, "no physical playback ports\n");
    exit (1);
  }

  // client, src, dst
  if (jack_connect (client, ports[0], jack_port_name (input_port1))) {
    fprintf (stderr, "cannot connect input ports\n");
  }

  if (jack_connect (client, ports[1], jack_port_name (input_port2))) {
    fprintf (stderr, "cannot connect input ports\n");
  }
  jack_free (ports);

  ports = jack_get_ports (client, NULL, NULL,
    		JackPortIsPhysical|JackPortIsInput);

  if (ports == NULL) {
    fprintf(stderr, "no physical playback ports\n");
    exit (1);
  }

  if (jack_connect (client, jack_port_name (output_port1), ports[0])) {
    fprintf (stderr, "cannot connect output ports\n");
  }

  if (jack_connect (client, jack_port_name (output_port2), ports[1])) {
    fprintf (stderr, "cannot connect output ports\n");
  }

  jack_free (ports);

    /* install a signal handler to properly quits jack client */
  signal(SIGQUIT, SignalHandler);
  signal(SIGTERM, SignalHandler);
  signal(SIGHUP, SignalHandler);
  signal(SIGINT, SignalHandler);

  /* keep running until the Ctrl+C */
  //jack_client_close(client);
  std::cout << "Jack Audio Init complete" << std::endl;
  return 0;
}
