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

void Test_Off_To_Record(TrackManager &tm) {
  std::cout << "** Test Off To Record **" << std::endl;
  // Default state - off
  tm.HandleDownEvent(0);
  bool result = tm.tracks.at(0).IsTrackInRecord();
  if (!result) {
    std::cout << "error: track not in record" << std::endl;
  }
}

void Test_Record_To_Playback(TrackManager &tm) {
  std::cout << "** Test Record to Play **" << std::endl;
  tm.HandleDownEvent(0);
  bool result = tm.tracks.at(0).IsTrackInPlayback();
  if (!result) {
    std::cout << "error: track not in playback" << std::endl;
  }
}

void Test_Playback_To_Overdub(TrackManager &tm) {
  std::cout << "** Test Play to Overdub **" << std::endl;
  tm.HandleDownEvent(0);
  bool result = tm.tracks.at(0).IsTrackOverdubbing();
  if (!result) {
    std::cout << "error: track not in overdubbing" << std::endl;
  }
}

void Test_Overdub_To_Playback(TrackManager &tm) {
  std::cout << "** Test Overdub to Play **" << std::endl;
  tm.HandleDownEvent(0);
  bool result = tm.tracks.at(0).IsTrackInPlayback();
  if (!result) {
    std::cout << "error: track not in playback" << std::endl;
  }
}

void Test_Playback_To_Mute(TrackManager &tm) {
  std::cout << "** Test Play to Mute **" << std::endl;
  tm.HandleDownEvent(0);
  tm.HandleDoubleDownEvent(0);
  bool result = tm.tracks.at(0).IsTrackMuted();
  if (!result) {
    std::cout << "error: track not muted" << std::endl;
  }
}

void Test_Mute_To_Playback(TrackManager &tm) {
  std::cout << "** Test Mute to Play **" << std::endl;
  tm.HandleDownEvent(0);
  bool result = tm.tracks.at(0).IsTrackInPlayback();
  if (!result) {
    std::cout << "error: track not in playback" << std::endl;
  }
}

void Test_Playback_To_Off(TrackManager &tm) {
  std::cout << "** Test Play to Off **" << std::endl;
  tm.HandleDoubleDownEvent(0);
  bool result = tm.tracks.at(0).IsTrackOff();
  if (!result) {
    std::cout << "error: track not off" << std::endl;
  }
}

void Test_Record_To_Repeat(TrackManager &tm) {
  std::cout << "** Test Record to Repeat **" << std::endl;
  tm.HandleLongPulseEvent(0);
  bool result = tm.tracks.at(0).IsTrackInPlaybackRepeat();
  if (!result) {
    std::cout << "error: track not in repeat" << std::endl;
  }
}

void Test_Repeat_To_Mute(TrackManager &tm) {
  std::cout << "** Test Repeat to Mute **" << std::endl;
  tm.HandleDownEvent(0);
  bool result = tm.tracks.at(0).IsTrackMuted();
  if (!result) {
    std::cout << "error: track not muted" << std::endl;
  }
}

void Test_Mute_To_Off(TrackManager &tm) {
  std::cout << "** Test Mute to Off **" << std::endl;
  tm.HandleDoubleDownEvent(0);
  bool result = tm.tracks.at(0).IsTrackOff();
  if (!result) {
    std::cout << "error: track not off" << std::endl;
  }
}

int main() {
  std::cout << "** test_state_machine.cpp **" << std::endl;
  Test_Off_To_Record(tm);
  Test_Record_To_Playback(tm);
  Test_Playback_To_Overdub(tm);
  Test_Overdub_To_Playback(tm);
  Test_Playback_To_Mute(tm);
  Test_Mute_To_Playback(tm);
  Test_Playback_To_Off(tm);
  Test_Off_To_Record(tm);
  Test_Record_To_Repeat(tm);
  Test_Repeat_To_Mute(tm);
  Test_Mute_To_Off(tm);
  return 0;
}
