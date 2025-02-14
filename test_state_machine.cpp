#include <array>
#include <iostream>
#include <iterator>
#include <chrono>
#include <thread>
#include <sys/time.h>    
#include "track_manager.h"

static TrackManager tm;

bool AreBlocksMatching(const DataBlock &expected, const DataBlock &test) {
  for (int i = 0; i < expected.samples.size(); i++) {
    if (expected.samples[i] != test.samples[i]) return false;
  }
  return true;
}

void DisplayIndexes(TrackManager &tm, uint32_t track_number) {
  std::cout << "TM:GMCI:" << tm.GetMasterCurrentIndex() << std::endl;
  std::cout << "TM:GMEI:" << tm.GetMasterEndIndex() << std::endl;
  std::cout << "TM:T:" << track_number << ":SI " << tm.tracks.at(track_number).GetStartIndex();
  std::cout << ":CI " << tm.tracks.at(track_number).GetCurrentIndex();
  std::cout << ":EI " << tm.tracks.at(track_number).GetEndIndex() << std::endl << std::endl;
}

bool DebugTesting(TrackManager &tm) {
  int idx = 0;
  bool result = true;
  std::cout << std::endl << "** Debug Testing **" << std::endl;

  for (idx = 0; idx < MAX_TRACK_COUNT; idx++) {
    tm.tracks.at(idx).IsTrackOff();
    if (!result) {
      std::cout << "error: track " << idx << " not off" << std::endl;
      return result;
    }
  }
  tm.HandleDownEvent(0);

  tm.HandleDownEvent(1);
  result = tm.tracks.at(0).IsTrackInPlayback();
  if (!result) {
    std::cout << "error: track 0 not in playback" << std::endl;
    return result;
  }
  result = tm.tracks.at(1).IsTrackInRecord();
  if (!result) {
    std::cout << "error: track 1 not in record" << std::endl;
    return result;
  }

  return result;
}

bool Test_NormalUseCases(TrackManager &tm) {
  std::cout << std::endl << "** Test Normal Use Cases - state machine **" << std::endl;

  std::cout << "    Test Off To Record **" << std::endl;
  // Default state - off
  tm.HandleDownEvent(0);
  bool result = tm.tracks.at(0).IsTrackInRecord();
  if (!result) {
    std::cout << "error: track not in record" << std::endl;
    return result;
  }

  std::cout << "    Test Record to Play **" << std::endl;
  tm.HandleDownEvent(0);
  result = tm.tracks.at(0).IsTrackInPlayback();
  if (!result) {
    std::cout << "error: track not in playback" << std::endl;
    return result;
  }

  std::cout << "    Test Play to Overdub **" << std::endl;
  tm.HandleDownEvent(0);
  result = tm.tracks.at(0).IsTrackOverdubbing();
  if (!result) {
    std::cout << "error: track not in overdubbing" << std::endl;
    return result;
  }

  std::cout << "    Test Overdub to Play **" << std::endl;
  tm.HandleDownEvent(0);
  result = tm.tracks.at(0).IsTrackInPlayback();
  if (!result) {
    std::cout << "error: track not in playback" << std::endl;
    return result;
  }

  std::cout << "    Test Play to Mute **" << std::endl;
  tm.HandleDownEvent(0);
  tm.HandleDoubleDownEvent(0);
  result = tm.tracks.at(0).IsTrackMuted();
  if (!result) {
    std::cout << "error: track not muted" << std::endl;
    return result;
  }

  std::cout << "    Test Mute to Play **" << std::endl;
  tm.HandleDownEvent(0);
  result = tm.tracks.at(0).IsTrackInPlayback();
  if (!result) {
    std::cout << "error: track not in playback" << std::endl;
    return result;
  }

  std::cout << "    Test Play to Off **" << std::endl;
  tm.HandleDoubleDownEvent(0);
  result = tm.tracks.at(0).IsTrackOff();
  if (!result) {
    std::cout << "error: track not off" << std::endl;
    return result;
  }

  std::cout << "    Test Off To Record **" << std::endl;
  // Default state - off
  tm.HandleDownEvent(0);
  result = tm.tracks.at(0).IsTrackInRecord();
  if (!result) {
    std::cout << "error: track not in record" << std::endl;
    return result;
  }

  std::cout << "    Test Record to Repeat **" << std::endl;
  // Off to Record
  tm.HandleLongPulseEvent(0);
  result = tm.tracks.at(0).IsTrackInPlaybackRepeat();
  if (!result) {
    std::cout << "error: track not in repeat" << std::endl;
    return result;
  }

  std::cout << "    Test Repeat to Mute **" << std::endl;
  tm.HandleDownEvent(0);
  result = tm.tracks.at(0).IsTrackMuted();
  if (!result) {
    std::cout << "error: track not muted" << std::endl;
    return result;
  }

  std::cout << "    Test Mute to Off **" << std::endl;
  tm.HandleDoubleDownEvent(0);
  result = tm.tracks.at(0).IsTrackOff();
  if (!result) {
    std::cout << "error: track not off" << std::endl;
    return result;
  }
  return result;
}

bool Test_AbnormalUseCases(TrackManager &tm) {
  std::cout << std::endl << "** Test Abnormal Use Cases - state machine **" << std::endl;
  // Starting from Off
  bool result = tm.tracks.at(0).IsTrackOff();
  if (!result) {
    std::cout << "error: track not off" << std::endl;
    return result;
  }

  std::cout << "    OFF:SP, LP, DD - expect no change" << std::endl;
  // Send Short Pulse, Long Pulse, and Double Down
  // Expect no change
  tm.HandleShortPulseEvent(0);
  result = tm.tracks.at(0).IsTrackOff();
  if (!result) {
    std::cout << "error: track not off" << std::endl;
    return result;
  }
  tm.HandleLongPulseEvent(0);
  result = tm.tracks.at(0).IsTrackOff();
  if (!result) {
    std::cout << "error: track not off" << std::endl;
    return result;
  }
  tm.HandleDoubleDownEvent(0);
  result = tm.tracks.at(0).IsTrackOff();
  if (!result) {
    std::cout << "error: track not off" << std::endl;
    return result;
  }

  std::cout << "    REC:SP, DD - expect no change" << std::endl;
  // Transition to Record
  tm.HandleDownEvent(0);
  result = tm.tracks.at(0).IsTrackInRecord();
  if (!result) {
    std::cout << "error: track not in record" << std::endl;
    return result;
  }
  // Send Short Pulse, Double Down
  // Expect no change
  std::cout << "    REC:SP, DD - expect no change" << std::endl;
  tm.HandleShortPulseEvent(0);
  result = tm.tracks.at(0).IsTrackInRecord();
  if (!result) {
    std::cout << "error: track not in Record" << std::endl;
    return result;
  }
  tm.HandleDoubleDownEvent(0);
  result = tm.tracks.at(0).IsTrackInRecord();
  if (!result) {
    std::cout << "error: track not in Record" << std::endl;
    return result;
  }

  std::cout << "    PLY:SP, LP - expect no change" << std::endl;
  // Transition to Play
  tm.HandleDownEvent(0);
  result = tm.tracks.at(0).IsTrackInPlayback();
  if (!result) {
    std::cout << "error: track not in playback" << std::endl;
    return result;
  }
  // Send Short Pulse and Long Pulse
  // Expect no change
  tm.HandleShortPulseEvent(0);
  result = tm.tracks.at(0).IsTrackInPlayback();
  if (!result) {
    std::cout << "error: track not in playback" << std::endl;
    return result;
  }
  tm.HandleLongPulseEvent(0);
  result = tm.tracks.at(0).IsTrackInPlayback();
  if (!result) {
    std::cout << "error: track not in playback" << std::endl;
    return result;
  }

  std::cout << "    OVD:SP - expect no change" << std::endl;
  // Transition to Overdub
  tm.HandleDownEvent(0);
  result = tm.tracks.at(0).IsTrackOverdubbing();
  if (!result) {
    std::cout << "error: track not in overdubbing" << std::endl;
    return result;
  }
  // Send Short Pulse
  // Expect no change
  tm.HandleShortPulseEvent(0);
  result = tm.tracks.at(0).IsTrackOverdubbing();
  if (!result) {
    std::cout << "error: track not in overdubbing" << std::endl;
    return result;
  }

  std::cout << "    RPT:SP, LP, DD - expect no change" << std::endl;
  // Transition to Repeat
  tm.HandleLongPulseEvent(0);
  result = tm.tracks.at(0).IsTrackInPlaybackRepeat();
  if (!result) {
    std::cout << "error: track not in repeat" << std::endl;
    return result;
  }
  // Send Short Pulse, Double Down, and Long Pulse
  // Expect no change
  tm.HandleShortPulseEvent(0);
  result = tm.tracks.at(0).IsTrackInPlaybackRepeat();
  if (!result) {
    std::cout << "error: track not in repeat" << std::endl;
    return result;
  }
  tm.HandleLongPulseEvent(0);
  result = tm.tracks.at(0).IsTrackInPlaybackRepeat();
  if (!result) {
    std::cout << "error: track not in repeat" << std::endl;
    return result;
  }
  tm.HandleDoubleDownEvent(0);
  result = tm.tracks.at(0).IsTrackInPlaybackRepeat();
  if (!result) {
    std::cout << "error: track not in repeat" << std::endl;
    return result;
  }

  std::cout << "    MUT:SP - expect no change" << std::endl;
  // Transition to Mute
  tm.HandleDownEvent(0);
  result = tm.tracks.at(0).IsTrackMuted();
  if (!result) {
    std::cout << "error: track not muted" << std::endl;
    return result;
  }
  // Send Short Pulse
  // Expect no change
  tm.HandleShortPulseEvent(0);
  result = tm.tracks.at(0).IsTrackMuted();
  if (!result) {
    std::cout << "error: track not muted" << std::endl;
    return result;
  }

  // Set track to Off
  std::cout << "    MUT:DD - expect change to Off **" << std::endl;
  tm.HandleDoubleDownEvent(0);
  result = tm.tracks.at(0).IsTrackOff();
  if (!result) {
    std::cout << "error: track not off" << std::endl;
    return result;
  }

  return result;
}

bool Test_TrackSwapWhileRecording(TrackManager &tm) {
  std::cout << std::endl << "** Test Track Swap While Recording - state machine **" << std::endl;
  // Starting from Off
  bool result = tm.tracks.at(0).IsTrackOff();
  if (!result) {
    std::cout << "error: track 0 not off" << std::endl;
    return result;
  }

  // Starting from Off
  result = tm.tracks.at(1).IsTrackOff();
  if (!result) {
    std::cout << "error: track 1 not off" << std::endl;
    return result;
  }

  // Start 0 recording
  std::cout << "    Record track 0 **" << std::endl;
  // Default state - off
  tm.HandleDownEvent(0);
  result = tm.tracks.at(0).IsTrackInRecord();
  if (!result) {
    std::cout << "error: track not in record" << std::endl;
    return result;
  }

  std::cout << "    Record track 1 -- Expect Track 0 to be set to playback" << std::endl;
  tm.HandleDownEvent(1);
  // Ensure track 1 in recording
  result = tm.tracks.at(1).IsTrackInRecord();
  if (!result) {
    std::cout << "error: track 1 not in record" << std::endl;
    return result;
  }

  result = tm.tracks.at(0).IsTrackInPlayback();
  if (!result) {
    std::cout << "error: track 0 not in playback" << std::endl;
    return result;
  }

  std::cout << "    Track 0 Play to Off, Track 1 is Rec so it should switch to Play" << std::endl;
  // set both to off
  tm.HandleDoubleDownEvent(0);
  result = tm.tracks.at(0).IsTrackOff();
  if (!result) {
    std::cout << "error: track 0 not off" << std::endl;
    return result;
  }

  result = tm.tracks.at(1).IsTrackInPlayback();
  if (!result) {
    std::cout << "error: track 0 not in playback" << std::endl;
    return result;
  }
  std::cout << "    Track 1 Play to Off" << std::endl;
  tm.HandleDoubleDownEvent(1);
  result = tm.tracks.at(1).IsTrackOff();
  if (!result) {
    std::cout << "error: track 1 not off" << std::endl;
    return result;
  }
  return result;
}

bool Test_TrackRecordingSwapWithIndexes(TrackManager &tm) {
  std::cout << std::endl << "** Test Track Swap While Recording Index Checks - state machine **" << std::endl;
  // Starting from Off
  bool result = tm.tracks.at(0).IsTrackOff();
  if (!result) {
    std::cout << "error: track 0 not off" << std::endl;
    return result;
  }

  // Starting from Off
  result = tm.tracks.at(1).IsTrackOff();
  if (!result) {
    std::cout << "error: track 1 not off" << std::endl;
    return result;
  }

  result = tm.GetMasterCurrentIndex() == 0; 
  if (!result) {
    std::cout << "error: master current index: " << tm.GetMasterCurrentIndex() << ", exp 0" << std::endl;
    return result;
  }

  result = tm.GetMasterEndIndex() == 0;
  if (!result) {
    std::cout << "error: master end index: " << tm.GetMasterEndIndex() << ", exp 0" << std::endl;
    return result;
  }


  // Start 0 recording
  std::cout << "    Record track 0 **" << std::endl;
  // Default state - off
  tm.HandleDownEvent(0);
  result = tm.tracks.at(0).IsTrackInRecord();
  if (!result) {
    std::cout << "error: track not in record" << std::endl;
    return result;
  }

  DisplayIndexes(tm, 0);
  std::cout << "    StateProcess 2x" << std::endl;
  tm.StateProcess(0); 
  tm.StateProcess(0);
  DisplayIndexes(tm, 0);

  std::cout << "    T0 R -> P" << std::endl;
  tm.HandleDownEvent(0);
  result = tm.tracks.at(0).IsTrackInPlayback();
  if (!result) {
    std::cout << "error: track 0 not in playback" << std::endl;
    return result;
  }
  DisplayIndexes(tm, 0);
  std::cout << "    T0 process 3x, 0->1->2->0->1->2" << std::endl;
  tm.StateProcess(0);
  DisplayIndexes(tm, 0);
  tm.StateProcess(0);
  DisplayIndexes(tm, 0);
  tm.StateProcess(0);
  DisplayIndexes(tm, 0);
  tm.StateProcess(0);
  DisplayIndexes(tm, 0);
  tm.StateProcess(0);
  DisplayIndexes(tm, 0);

  std::cout << "    Record track 1" << std::endl;
  tm.HandleDownEvent(1);
  // Ensure track 1 in recording
  result = tm.tracks.at(1).IsTrackInRecord();
  if (!result) {
    std::cout << "error: track 1 not in record" << std::endl;
    return result;
  }

  result = tm.tracks.at(0).IsTrackInPlayback();
  if (!result) {
    std::cout << "error: track 0 not in playback" << std::endl;
    return result;
  }

  std::cout << "    T1 Rec, T0 Playback" << std::endl;
  DisplayIndexes(tm, 0);
  DisplayIndexes(tm, 1);

  std::cout << "    T1 process 2x" << std::endl;
  tm.StateProcess(1); 
  DisplayIndexes(tm, 0);
  DisplayIndexes(tm, 1);
  tm.StateProcess(1);
  DisplayIndexes(tm, 0);
  DisplayIndexes(tm, 1);

  std::cout << "    Track 0 Play to Off, Track 1 is Rec so it should switch to Play" << std::endl;
  // set both to off
  tm.HandleDoubleDownEvent(0);
  result = tm.tracks.at(0).IsTrackOff();
  if (!result) {
    std::cout << "error: track 0 not off" << std::endl;
    return result;
  }

  result = tm.tracks.at(1).IsTrackInPlayback();
  if (!result) {
    std::cout << "error: track 0 not in playback" << std::endl;
    return result;
  }
  std::cout << "    Track 1 Play to Off" << std::endl;
  tm.HandleDoubleDownEvent(1);
  result = tm.tracks.at(1).IsTrackOff();
  if (!result) {
    std::cout << "error: track 1 not off" << std::endl;
    return result;
  }
  DisplayIndexes(tm, 0);
  DisplayIndexes(tm, 1);

  return result;
}

void Test_TrackRecordingNewTrackPlayToOff(TrackManager &tm) {
}

void Test_RecordingSingleTrackWithData(TrackManager &tm) {
}

int main() {
  std::cout << "** test_state_machine.cpp **" << std::endl;
#if 0
  bool result = DebugTesting(tm);
  if (!result) {
    std::cout << "---> TEST FAILED" << std::endl;
  }
#else
  bool result = Test_NormalUseCases(tm);
  if (!result) {
    std::cout << "---> TEST FAILED" << std::endl;
  }

  result = Test_AbnormalUseCases(tm);
  if (!result) {
    std::cout << "---> TEST FAILED" << std::endl;
  }

  result = Test_TrackSwapWhileRecording(tm);
  if (!result) {
    std::cout << "---> TEST FAILED" << std::endl;
  }
  result = Test_TrackRecordingSwapWithIndexes(tm);
  if (!result) {
    std::cout << "---> TEST FAILED" << std::endl;
  }

#endif

  return 0;
}
