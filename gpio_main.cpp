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
#include "track_manager.h"
#include "group_manager.h"
#include "input_gpio.h"
#include "output_i2c.h"
#include "audio_jack.h"

static InputGpio gi;
static OutputI2C oi;
static TrackManager tm;
static GroupManager gm;
static AudioJack jack;

int
main (int argc, char *argv[])
{

  jack.SetTrackManagerPtr(&tm, nullptr);
  jack.SetInputGpioPtr(&gi);
  jack.Init(argc, argv);

//int main() {
  std::cout << "Initializing WiringPi GPIO - this takes a long time" << std::endl;
  if (!gi.InitializeWiringPiGpio()) {
    return 1;
  }
  std::cout << "Initializing WiringPi I2C" << std::endl;
  if (!oi.InitializeWiringPiI2C()) {
    return 1;
  }

  tm.SetOutputI2CPtr(&oi);
  gm.SetOutputI2CPtr(&oi);

  struct timeval tstart, tend, tdiff;
  auto th_id = std::this_thread::get_id();
  std::cout << "MainThread ID, "<< th_id << std::endl;

  gi.Reset();
  std::this_thread::sleep_for(std::chrono::milliseconds(100));

  std::cout << "Enable Jack Audio Processing" << std::endl;
  jack.EnableJackAudioProcessing();

  std::cout << "Entering while1" << std::endl;

  // Hammer on this but need to handle multiple events case!
  while(1) {
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
    if (gi.ProcessAndHandleInputEvents()) {
      gettimeofday(&tstart, NULL);
      if (gm.IsStateAddTrack() && gi.LastEventWasDown() && gi.LastEventWasForTrack()) {
        //gm.AddTrackToGroup(gi.GetLastTrack(), gi.GetLastGroup());
	// This adds tracks to group - redundant but in gpio should only send the events
	// it doesn't have to know about the innerworkings of events
        gm.StateProcess(tm, gi.GetLastGroup(), gi.GetLastTrack());
	gm.DisplayGroups();
      } else if (gm.IsStateRemoveTracks() && gi.LastEventWasDown() && gi.LastEventWasForTrack()) {
        //gm.AddTrackToGroup(gi.GetLastTrack(), gi.GetLastGroup());
	// This adds tracks to group - redundant but in gpio should only send the events
	// it doesn't have to know about the innerworkings of events
        gm.StateProcess(tm, gi.GetLastGroup(), gi.GetLastTrack());
	gm.DisplayGroups();
      } else {

      // Use IsTrackMemberOfGroup to prevent tracks from other groups interfering with
      // active group
      if (gi.LastEventWasDown()) {
        if (gi.LastEventWasForTrack()) {
	  if (gm.IsTrackMemberOfGroup(gi.GetLastTrack(), gi.GetLastGroup())) {
            std::cout << "E:Down, T:" << gi.GetLastTrack() << std::endl;
	    tm.HandleDownEvent(gi.GetLastTrack());
	  }
        } else {
          std::cout << "E:Down, G:" << gi.GetLastGroup() << std::endl;
	  gm.HandleDownEvent(tm, gi.GetLastGroup(), gi.GetLastTrack());
        }
      }
      if (gi.LastEventWasDoubleDown()) {
        if (gi.LastEventWasForTrack()) {
	  if (gm.IsTrackMemberOfGroup(gi.GetLastTrack(), gi.GetLastGroup())) {
            std::cout << "E:DoubleDown, T:" << gi.GetLastTrack() << std::endl;
	    tm.HandleDoubleDownEvent(gi.GetLastTrack());
	  }
        } else {
          std::cout << "E:DoubleDown, G:" << gi.GetLastGroup() << std::endl;
	  gm.HandleDoubleDownEvent(tm, gi.GetLastGroup(), gi.GetLastTrack());
        }
      }
      if (gi.LastEventWasLongPulse()) {
        if (gi.LastEventWasForTrack()) {
	  if (gm.IsTrackMemberOfGroup(gi.GetLastTrack(), gi.GetLastGroup())) {
            std::cout << "E:LongPulse, T:" << gi.GetLastTrack() << std::endl;
	    tm.HandleLongPulseEvent(gi.GetLastTrack());
	  }
        } else {
          std::cout << "E:LongPulse, G:" << gi.GetLastGroup() << std::endl;
	  gm.HandleLongPulseEvent(tm, gi.GetLastGroup(), gi.GetLastTrack());
        }
      }

      }
      gettimeofday(&tend, NULL);
      timersub(&tend, &tstart, &tdiff);
      std::cout << "Processing Time S:" << std::dec << tdiff.tv_sec << ":" << std::dec << tdiff.tv_usec << std::endl;
    }
  }

  return 0;
}
