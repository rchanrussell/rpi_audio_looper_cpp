#include <array>
#include <iostream>
#include <iterator>
#include <chrono>
#include <thread>
#include <sys/time.h>    
#include <unistd.h>
#include "track_manager.h"

static TrackManager tm;

bool AreBlocksMatching(const DataBlock &expected, const DataBlock &test) {
  for (uint32_t i = 0; i < expected.samples_.size(); i++) {
    if (expected.samples_[i] != test.samples_[i]) return false;
  }
  return true;
}

struct Indexes {
  uint32_t gmci;
  uint32_t gmei;
  uint32_t t_ci;
  uint32_t t_ei;
  uint32_t t_si;
};

bool VerifyIndexes(TrackManager &tm, struct Indexes e, uint32_t track_num) {
  uint32_t idx = tm.GetMasterCurrentIndex();
  bool result = idx == e.gmci; 
  if (!result) {
    std::cout << "error: master current index: " << idx << ", exp:" << e.gmci << std::endl;
    return result;
  }

  idx = tm.GetMasterEndIndex();
  result = idx == e.gmei;
  if (!result) {
    std::cout << "error: master end index: " << idx << ", exp:" << e.gmei << std::endl;
    return result;
  }

  idx = tm.tracks.at(track_num).GetStartIndex();
  result = idx == e.t_si;
  if (!result) {
    std::cout << "error: track " << track_num << " start index: " << idx << ", exp:" << e.t_si << std::endl;
    return result;
  }

  idx = tm.tracks.at(track_num).GetCurrentIndex();
  result = idx == e.t_ci;
  if (!result) {
    std::cout << "error: track " << track_num << " current index: " << idx << ", exp:" << e.t_ci << std::endl;
    return result;
  }

  idx = tm.tracks.at(track_num).GetEndIndex();
  result = idx == e.t_ei;
  if (!result) {
    std::cout << "error: track " << track_num << " end index: " << idx << ", exp:" << e.t_ei << std::endl;
    return result;
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
  // set t0 to off
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

bool Test_SingleTrackRecordingToPlaybackIndexes(TrackManager &tm) {
  std::cout << std::endl << "** Test Single Track Recording To Playback with Index Checks - state machine **" << std::endl;

  struct Indexes exp = {0, 0, 0, 0, 0};
  // Starting from Off
  bool result = tm.tracks.at(0).IsTrackOff();
  if (!result) {
    std::cout << "error: track 0 not off" << std::endl;
    return result;
  }
  if (!VerifyIndexes(tm, exp, 0)) { return false; }

  // Start 0 recording
  std::cout << "    Record track 0 **" << std::endl;
  // Default state - off
  tm.HandleDownEvent(0);
  result = tm.tracks.at(0).IsTrackInRecord();
  if (!result) {
    std::cout << "error: track not in record" << std::endl;
    return result;
  }

  if (!VerifyIndexes(tm, exp, 0)) { return false; }

  DisplayIndexes(tm, 0);
  std::cout << "    StateProcess 2x" << std::endl;
  tm.StateProcess(0);
  tm.StateProcess(0);

  exp.gmci = 2;
  exp.gmei = 0;
  exp.t_ci = 2;
  exp.t_ei = 0;
  DisplayIndexes(tm, 0);
  if (!VerifyIndexes(tm, exp, 0)) { return false; }

  std::cout << "    T0 R -> P" << std::endl;
  tm.HandleDownEvent(0);
  result = tm.tracks.at(0).IsTrackInPlayback();
  if (!result) {
    std::cout << "error: track 0 not in playback" << std::endl;
    return result;
  }

  DisplayIndexes(tm, 0);
  exp.gmei = 2;
  exp.t_ei = 2;
  exp.gmci = 0;
  exp.t_ci = 0;
  if (!VerifyIndexes(tm, exp, 0)) { return false; }

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

  exp.gmei = 2;
  exp.t_ei = 2;
  exp.gmci = 2;
  exp.t_ci = 2;
  if (!VerifyIndexes(tm, exp, 0)) { return false; }

  std::cout << "    T0 P -> Off" << std::endl;
  tm.HandleDoubleDownEvent(0);
  result = tm.tracks.at(0).IsTrackOff();
  if (!result) {
    std::cout << "error: track 0 not in off" << std::endl;
    return result;
  }

  DisplayIndexes(tm, 0);
  exp.t_ei = 0;
  exp.t_ci = 0;
  exp.t_si = 0;
  exp.gmci = 0;
  exp.gmei = 0;
  if (!VerifyIndexes(tm, exp, 0)) { return false; }

  return true;
}

bool Test_TrackRecordingNewTrackStartsRecordingIndexes(TrackManager &tm) {
  std::cout << std::endl << "** Test Track recording then new track starts recording with Index Checks - state machine **" << std::endl;

  struct Indexes exp = {0, 0, 0, 0, 0};
  // Starting from Off
  bool result = tm.tracks.at(0).IsTrackOff();
  if (!result) {
    std::cout << "error: track 0 not off" << std::endl;
    return result;
  }
  if (!VerifyIndexes(tm, exp, 0)) { return false; }

  result = tm.tracks.at(1).IsTrackOff();
  if (!result) {
    std::cout << "error: track 1 not off" << std::endl;
    return result;
  }
  if (!VerifyIndexes(tm, exp, 1)) { return false; }

  // Start 0 recording
  std::cout << "    Record track 0 **" << std::endl;
  // Default state - off
  tm.HandleDownEvent(0);
  result = tm.tracks.at(0).IsTrackInRecord();
  if (!result) {
    std::cout << "error: track not in record" << std::endl;
    return result;
  }

  if (!VerifyIndexes(tm, exp, 0)) { return false; }

  DisplayIndexes(tm, 0);
  std::cout << "    StateProcess 2x" << std::endl;
  tm.StateProcess(0);
  tm.StateProcess(0);

  exp.gmci = 2;
  exp.gmei = 0;
  exp.t_ci = 2;
  exp.t_ei = 0;
  DisplayIndexes(tm, 0);
  if (!VerifyIndexes(tm, exp, 0)) { return false; }

  std::cout << "    Press record on track 1" << std::endl;
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

  // Because T0 when to playback first, this will reset master current index to 0
  exp.gmci = 0;
  exp.gmei = 2;
  // T0
  exp.t_ci = 0;
  exp.t_ei = 2;
  exp.t_si = 0;
  if (!VerifyIndexes(tm, exp, 0)) { return false; }

  // T1
  exp.t_ci = 0;
  exp.t_ei = 0;
  exp.t_si = 0;
  if (!VerifyIndexes(tm, exp, 1)) { return false; }

  std::cout << "    StateProcess 2x" << std::endl;
  tm.StateProcess(1);
  tm.StateProcess(1);

  exp.gmci = 2;
  exp.gmei = 2;
  // T0
  exp.t_ci = 2;
  exp.t_ei = 2;
  exp.t_si = 0;
  if (!VerifyIndexes(tm, exp, 0)) { return false; }

  // T1
  exp.t_ci = 2;
  exp.t_ei = 0;
  exp.t_si = 0;
  if (!VerifyIndexes(tm, exp, 1)) { return false; }

  std::cout << "    T0 Play to Off, T1 is Rec so it should switch to Play" << std::endl;
  // Set T0 Off
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

  exp.gmci = 0;
  exp.gmei = 2;
  // T0
  exp.t_ci = 0;
  exp.t_ei = 0;
  exp.t_si = 0;
  if (!VerifyIndexes(tm, exp, 0)) { return false; }

  // T1
  exp.t_ci = 0;
  exp.t_ei = 2;
  exp.t_si = 0;
  if (!VerifyIndexes(tm, exp, 1)) { return false; }


  std::cout << "    Track 1 Play to Off" << std::endl;
  tm.HandleDoubleDownEvent(1);
  result = tm.tracks.at(1).IsTrackOff();
  if (!result) {
    std::cout << "error: track 1 not off" << std::endl;
    return result;
  }

  exp.gmci = 0;
  exp.gmei = 0;
  // T1
  exp.t_ci = 0;
  exp.t_ei = 0;
  exp.t_si = 0;
  if (!VerifyIndexes(tm, exp, 1)) { return false; }

  return true;
}

bool Test_TrackPlaybackNewTrackStartsRecordingIndexes(TrackManager &tm) {
  std::cout << std::endl << "** Test Track in playback then new track starts recording with Index Checks - state machine **" << std::endl;

  struct Indexes exp = {0, 0, 0, 0, 0};
  // Starting from Off
  bool result = tm.tracks.at(0).IsTrackOff();
  if (!result) {
    std::cout << "error: track 0 not off" << std::endl;
    return result;
  }
  if (!VerifyIndexes(tm, exp, 0)) { return false; }

  result = tm.tracks.at(1).IsTrackOff();
  if (!result) {
    std::cout << "error: track 1 not off" << std::endl;
    return result;
  }
  if (!VerifyIndexes(tm, exp, 1)) { return false; }

  // Start 0 recording
  std::cout << "    Record track 0 **" << std::endl;
  // Default state - off
  tm.HandleDownEvent(0);
  result = tm.tracks.at(0).IsTrackInRecord();
  if (!result) {
    std::cout << "error: track not in record" << std::endl;
    return result;
  }

  if (!VerifyIndexes(tm, exp, 0)) { return false; }

  DisplayIndexes(tm, 0);
  std::cout << "    StateProcess 2x" << std::endl;
  tm.StateProcess(0);
  tm.StateProcess(0);

  exp.gmci = 2;
  exp.gmei = 0;
  exp.t_ci = 2;
  exp.t_ei = 0;
  DisplayIndexes(tm, 0);
  if (!VerifyIndexes(tm, exp, 0)) { return false; }

  std::cout << "    Set track 0 to playback" << std::endl;
  tm.HandleDownEvent(0);
  result = tm.tracks.at(0).IsTrackInPlayback();
  if (!result) {
    std::cout << "error: track 0 not in playback" << std::endl;
    return result;
  }

  DisplayIndexes(tm, 0);
  exp.gmei = 2;
  exp.t_ei = 2;
  exp.gmci = 0;
  exp.t_ci = 0;
  if (!VerifyIndexes(tm, exp, 0)) { return false; }

  // Play 1 block then start recording on T1
  tm.StateProcess(0);

  exp.gmei = 2;
  exp.gmci = 1;
  exp.t_ci = 1;
  exp.t_ei = 2;
  exp.t_si = 0;
  if (!VerifyIndexes(tm, exp, 0)) { return false; }

  std::cout << "    Press record on track 1" << std::endl;
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

  exp.gmci = 1;
  exp.gmei = 2;
  // T0
  exp.t_ci = 1;
  exp.t_ei = 2;
  exp.t_si = 0;
  if (!VerifyIndexes(tm, exp, 0)) { return false; }

  // T1
  exp.t_ci = 1;
  exp.t_ei = 0;
  exp.t_si = 1;
  if (!VerifyIndexes(tm, exp, 1)) { return false; }

  std::cout << "    StateProcess 2x" << std::endl;
  tm.StateProcess(1);
  tm.StateProcess(1);

  DisplayIndexes(tm, 0);
  DisplayIndexes(tm, 1);

  exp.gmci = 3;
  exp.gmei = 2;
  // T0
  exp.t_ci = 3;
  exp.t_ei = 2;
  exp.t_si = 0;
  if (!VerifyIndexes(tm, exp, 0)) { return false; }

  // T1
  exp.t_ci = 3;
  exp.t_ei = 0;
  exp.t_si = 1;
  if (!VerifyIndexes(tm, exp, 1)) { return false; }

  std::cout << "    T1 to play" << std::endl;
  tm.HandleDownEvent(1);
  result = tm.tracks.at(1).IsTrackInPlayback();
  if (!result) {
    std::cout << "error: track 1 not in playback" << std::endl;
    return result;
  }

  DisplayIndexes(tm, 0);
  DisplayIndexes(tm, 1);
  exp.gmci = 0;
  exp.gmei = 3;
  // T0
  exp.t_ci = 3; // T1 to play does no reset T0's current index -- PerformMixdown
  // does not use track's current index, uses master current index unless in repeat
  // so this is to be expected, playback doesn't use it, only repeat does
  exp.t_ei = 2;
  exp.t_si = 0;
  if (!VerifyIndexes(tm, exp, 0)) { return false; }

  // T1
  exp.t_ci = 0;
  exp.t_ei = 3;
  exp.t_si = 1;
  if (!VerifyIndexes(tm, exp, 1)) { return false; }

  DisplayIndexes(tm, 0);
  DisplayIndexes(tm, 1);
  std::cout << "    SP1" << std::endl;
  tm.StateProcess(1);
  DisplayIndexes(tm, 0);
  DisplayIndexes(tm, 1);
  std::cout << "    SP1" << std::endl;
  tm.StateProcess(1);
  DisplayIndexes(tm, 0);
  DisplayIndexes(tm, 1);
  std::cout << "    SP0" << std::endl;
  tm.StateProcess(0);
  DisplayIndexes(tm, 0);
  DisplayIndexes(tm, 1);
  std::cout << "    SP0" << std::endl;
  tm.StateProcess(1);
  DisplayIndexes(tm, 0);
  DisplayIndexes(tm, 1);



  std::cout << "    T0 to Off" << std::endl;
  // Set T0 Off
  tm.HandleDoubleDownEvent(0);
  result = tm.tracks.at(0).IsTrackOff();
  if (!result) {
    std::cout << "error: track 0 not off" << std::endl;
    return result;
  }

  exp.gmci = 0;
  exp.gmei = 3;
  // T0
  exp.t_ci = 0;
  exp.t_ei = 0;
  exp.t_si = 0;
  if (!VerifyIndexes(tm, exp, 0)) { return false; }

  // T1
  exp.t_ci = 4;
  exp.t_ei = 3;
  exp.t_si = 1;
  if (!VerifyIndexes(tm, exp, 1)) { return false; }

  result = tm.tracks.at(1).IsTrackInPlayback();
  if (!result) {
    std::cout << "error: track 1 not in playback" << std::endl;
    return result;
  }

  std::cout << "   T1 to off" << std::endl;
  tm.HandleDoubleDownEvent(1);
  result = tm.tracks.at(1).IsTrackOff();
  if (!result) {
    std::cout << "error: track 1 not off" << std::endl;
    return result;
  }

  exp.gmci = 0;
  exp.gmei = 0;
  // T0
  exp.t_ci = 0;
  exp.t_ei = 0;
  exp.t_si = 0;
  if (!VerifyIndexes(tm, exp, 0)) { return false; }

  // T1
  exp.t_ci = 0;
  exp.t_ei = 0;
  exp.t_si = 0;
  if (!VerifyIndexes(tm, exp, 1)) { return false; }
  return true;
}

bool Test_TrackPlaybackNewTrackStartsRecordingThenRepeatsIndexes(TrackManager &tm) {
  std::cout << std::endl << "** Test Track in playback then new track starts recording then repeats with Index Checks - state machine **" << std::endl;

  struct Indexes exp = {0, 0, 0, 0, 0};
  // Starting from Off
  bool result = tm.tracks.at(0).IsTrackOff();
  if (!result) {
    std::cout << "error: track 0 not off" << std::endl;
    return result;
  }
  if (!VerifyIndexes(tm, exp, 0)) { return false; }

  result = tm.tracks.at(1).IsTrackOff();
  if (!result) {
    std::cout << "error: track 1 not off" << std::endl;
    return result;
  }
  if (!VerifyIndexes(tm, exp, 1)) { return false; }

  // Start 0 recording
  std::cout << "    Record track 0 **" << std::endl;
  // Default state - off
  tm.HandleDownEvent(0);
  result = tm.tracks.at(0).IsTrackInRecord();
  if (!result) {
    std::cout << "error: track not in record" << std::endl;
    return result;
  }

  if (!VerifyIndexes(tm, exp, 0)) { return false; }

  DisplayIndexes(tm, 0);
  std::cout << "    StateProcess 2x" << std::endl;
  tm.StateProcess(0);
  tm.StateProcess(0);

  exp.gmci = 2;
  exp.gmei = 0;
  exp.t_ci = 2;
  exp.t_ei = 0;
  DisplayIndexes(tm, 0);
  if (!VerifyIndexes(tm, exp, 0)) { return false; }

  std::cout << "    Set track 0 to playback" << std::endl;
  tm.HandleDownEvent(0);
  result = tm.tracks.at(0).IsTrackInPlayback();
  if (!result) {
    std::cout << "error: track 0 not in playback" << std::endl;
    return result;
  }

  DisplayIndexes(tm, 0);
  exp.gmei = 2;
  exp.t_ei = 2;
  exp.gmci = 0;
  exp.t_ci = 0;
  if (!VerifyIndexes(tm, exp, 0)) { return false; }

  // Play 1 block then start recording on T1
  std::cout << "    Process t0 in play" << std::endl;
  tm.StateProcess(0);

  exp.gmei = 2;
  exp.gmci = 1;
  exp.t_ci = 1;
  exp.t_ei = 2;
  exp.t_si = 0;
  if (!VerifyIndexes(tm, exp, 0)) { return false; }

  std::cout << "    Press record on track 1" << std::endl;
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

  exp.gmci = 1;
  exp.gmei = 2;
  // T0
  exp.t_ci = 1;
  exp.t_ei = 2;
  exp.t_si = 0;
  if (!VerifyIndexes(tm, exp, 0)) { return false; }

  // T1
  exp.t_ci = 1;
  exp.t_ei = 0;
  exp.t_si = 1;
  if (!VerifyIndexes(tm, exp, 1)) { return false; }

  std::cout << "    StateProcess 2x" << std::endl;
  tm.StateProcess(1);
  tm.StateProcess(1);

  DisplayIndexes(tm, 0);
  DisplayIndexes(tm, 1);

  exp.gmci = 3;
  exp.gmei = 2;
  // T0
  exp.t_ci = 3;
  exp.t_ei = 2;
  exp.t_si = 0;
  if (!VerifyIndexes(tm, exp, 0)) { return false; }

  // T1
  exp.t_ci = 3;
  exp.t_ei = 0;
  exp.t_si = 1;
  if (!VerifyIndexes(tm, exp, 1)) { return false; }

  std::cout << "    T1 to repeat" << std::endl;
  tm.HandleLongPulseEvent(1);
  result = tm.tracks.at(1).IsTrackInPlaybackRepeat();
  if (!result) {
    std::cout << "error: track 1 not in playback repeat" << std::endl;
    return result;
  }

  DisplayIndexes(tm, 0);
  DisplayIndexes(tm, 1);
  exp.gmci = 0;
  exp.gmei = 3;
  // T0
  exp.t_ci = 3; // T1 to play does no reset T0's current index -- PerformMixdown
  // does not use track's current index, uses master current index unless in repeat
  // so this is to be expected, playback doesn't use it, only repeat does
  exp.t_ei = 2;
  exp.t_si = 0;
  if (!VerifyIndexes(tm, exp, 0)) { return false; }

  // T1
  exp.t_ci = 1; // repeat - CI is sync'd with SI not MCI
  exp.t_ei = 3;
  exp.t_si = 1;
  if (!VerifyIndexes(tm, exp, 1)) { return false; }

  DisplayIndexes(tm, 0);
  DisplayIndexes(tm, 1);
  std::cout << "    SP1" << std::endl;
  tm.StateProcess(1);
  DisplayIndexes(tm, 0);
  DisplayIndexes(tm, 1);
  std::cout << "    SP1" << std::endl;
  tm.StateProcess(1);
  DisplayIndexes(tm, 0);
  DisplayIndexes(tm, 1);
  std::cout << "    SP0" << std::endl;
  tm.StateProcess(0);
  DisplayIndexes(tm, 0);
  DisplayIndexes(tm, 1);
  std::cout << "    SP0" << std::endl;
  tm.StateProcess(1);
  DisplayIndexes(tm, 0);
  DisplayIndexes(tm, 1);



  std::cout << "    T0 to Off" << std::endl;
  // Set T0 Off
  tm.HandleDoubleDownEvent(0);
  result = tm.tracks.at(0).IsTrackOff();
  if (!result) {
    std::cout << "error: track 0 not off" << std::endl;
    return result;
  }

  exp.gmci = 0;
  exp.gmei = 3;
  // T0
  exp.t_ci = 0;
  exp.t_ei = 0;
  exp.t_si = 0;
  if (!VerifyIndexes(tm, exp, 0)) { return false; }

  // T1
  exp.t_ci = 2;
  exp.t_ei = 3;
  exp.t_si = 1;
  if (!VerifyIndexes(tm, exp, 1)) { return false; }

  result = tm.tracks.at(1).IsTrackInPlaybackRepeat();
  if (!result) {
    std::cout << "error: track 1 not in playback repeat" << std::endl;
    return result;
  }

  std::cout << "   T1 3 repeat process" << std::endl;
  tm.StateProcess(1);
  DisplayIndexes(tm, 1);
  tm.StateProcess(1);
  DisplayIndexes(tm, 1);
  tm.StateProcess(1);
  DisplayIndexes(tm, 1);

  std::cout << "   T1 to off" << std::endl;
  tm.HandleDownEvent(1); // To Mute
  tm.HandleDoubleDownEvent(1); // To Off
  result = tm.tracks.at(1).IsTrackOff();
  if (!result) {
    std::cout << "error: track 1 not off" << std::endl;
    return result;
  }

  exp.gmci = 0;
  exp.gmei = 0;
  // T0
  exp.t_ci = 0;
  exp.t_ei = 0;
  exp.t_si = 0;
  if (!VerifyIndexes(tm, exp, 0)) { return false; }

  // T1
  exp.t_ci = 0;
  exp.t_ei = 0;
  exp.t_si = 0;
  if (!VerifyIndexes(tm, exp, 1)) { return false; }
  return true;
}

void Test_TrackRecordingNewTrackPlayToOff(TrackManager &tm) {
}

bool Test_RecordingSingleTrackWithData(TrackManager &tm) {
  std::cout << std::endl << "** Test Recording with data - state machine **" << std::endl;

  bool result = tm.tracks.at(0).IsTrackOff();
  if (!result) {
    std::cout << "error: track 0 not off" << std::endl;
    return result;
  }

  result = tm.tracks.at(1).IsTrackOff();
  if (!result) {
    std::cout << "error: track 1 not off" << std::endl;
    return result;
  }

  std::cout << "    Input set" << std::endl;
  // Input
  //int *ptr =(int *)calloc(1, 1024);
  int ptr[1024];
  //if (!ptr) { std::cout << "input buf Calloc failed" << std::endl; return false; }
  uint8_t val = 1; 
  for (int idx=0; idx<1024; idx++) {
    if (idx > 0 && (idx % 128 == 0)) { val++; }
    ptr[idx] = val;
  }

  // Output
  //int *pm =(int *)calloc(1, 1024);
  int pm[1024];
  //if (!pm) { std::cout << "output buff Calloc failed" << std::endl; return false; }


  // Start 0 recording
  std::cout << "    Record track 0 **" << std::endl;
  // Default state - off
  tm.HandleDownEvent(0);
  result = tm.tracks.at(0).IsTrackInRecord();
  if (!result) {
    std::cout << "error: track not in record" << std::endl;
    return result;
  }

  struct timeval t1, t2, tdiff;
  gettimeofday(&t1, NULL);

  // Everytime Process callback (JackAudio) or the input buffer is ready
  // do the following three steps
  // for JackAudio, ptr and pm will actually be jack ports
  // the other handle events will be called by the IO system (currently in
  // it is in a functional state in no-source controlled code

  tm.CopyToInputBuffer(ptr, SAMPLES_PER_BLOCK);
  tm.StateProcess(0);
  tm.CopyMixdownToBuffer(pm, 128);

  gettimeofday(&t2, NULL);

  timersub(&t2, &t1, &tdiff);
  std::cout << "td: " << tdiff.tv_sec << "," << tdiff.tv_usec << std::endl;
  //mci,mei,tci,tei,tsi
  struct Indexes e = { 1,0,1,0,0};

  if(!VerifyIndexes(tm, e, 0)) { return false; }

  for (int idx=0; idx<128; idx++) {
    if (pm[idx] != ptr[idx]) {
      std::cout << "error: pm[" << idx << "]:" << pm[idx] << " =/= " << " ptr[" << idx << "]:" << ptr[idx] << std::endl;
      return false;
    }
  }
  std::cout << std::endl;

  // transition to play
  tm.HandleDownEvent(0);
  tm.HandleDoubleDownEvent(0); 
  result = tm.tracks.at(0).IsTrackOff();
  if (!result) {
    std::cout << "error: track 0 not off" << std::endl;
    return result;
  }

  //free(ptr);
  //free(pm);
  return true;
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

  result = Test_SingleTrackRecordingToPlaybackIndexes(tm);
  if (!result) {
    std::cout << "---> TEST FAILED" << std::endl;
  }

  result = Test_TrackRecordingNewTrackStartsRecordingIndexes(tm);
  if (!result) {
    std::cout << "---> TEST FAILED" << std::endl;
  }

  result = Test_TrackPlaybackNewTrackStartsRecordingIndexes(tm);
  if (!result) {
    std::cout << "---> TEST FAILED" << std::endl;
  }

  result = Test_TrackPlaybackNewTrackStartsRecordingThenRepeatsIndexes(tm);
  if (!result) {
    std::cout << "---> TEST FAILED" << std::endl;
  }

  result = Test_RecordingSingleTrackWithData(tm);
  if (!result) {
    std::cout << "---> TEST FAILED" << std::endl;
  }

#endif

  return 0;
}
