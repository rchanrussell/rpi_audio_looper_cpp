#include <array>
#include <iostream>
#include <iterator>
#include <sys/time.h>    
#include "group_manager.h"

static TrackManager tm;


bool AreBlocksMatching(const DataBlock &expected, const DataBlock &test) {
  for (uint32_t i = 0; i < expected.samples_.size(); i++) {
    if (expected.samples_[i] != test.samples_[i]) return false;
  }
  return true;
}

void DisplayTrackMuted(TrackManager &tm) {
  std::cout << "TGM::DTM Is Muted?" << std::endl;
  for (auto &t : tm.tracks) {
    std::cout << t.IsTrackMuted() << " ";
  }
  std::cout << std::endl << "    master end index " << tm.GetMasterEndIndex() << std::endl;
}

void DisplayTrackInPlayback(TrackManager &tm) {
  std::cout << "TGM::DTIP Is Playing?" << std::endl;
  for (auto &t : tm.tracks) {
    std::cout << t.IsTrackInPlayback() << " ";
  }
  std::cout << std::endl;
}

void Test_AddTracksToGroups(GroupManager &gm, TrackManager &tm) {
  std::cout << "** test_group_manager.cpp: Add Tracks to Groups **" << std::endl;
  gm.DisplayGroups();

  std::cout << "    active_group " << unsigned(gm.GetActiveGroup()) << std::endl;

  gm.AddTrackToGroup(5, 2);
  gm.AddTrackToGroup(3, 2);
  gm.AddTrackToGroup(1, 2);
  gm.AddTrackToGroup(2, 1);
  gm.AddTrackToGroup(4, 1);
  gm.AddTrackToGroup(6, 1);
  gm.DisplayGroups();
}

void Test_RemoveTracksFromGroups(GroupManager &gm, TrackManager &tm) {
  std::cout << std::endl << std::endl << "** test_group_manager.cpp: Remove Tracks From Groups **" << std::endl;
  gm.DisplayGroups();

  // Exp no change
  gm.RemoveTrackFromGroup(6, 2);
  gm.DisplayGroups();

  // Exp change
  gm.RemoveTrackFromGroup(1, 2);
  gm.DisplayGroups();

}

void Test_ChangeActiveGroups(GroupManager &gm, TrackManager &tm) {
  std::cout << std::endl << std::endl << "** test_group_manager.cpp: Change Active Groups **" << std::endl;
  gm.AddTrackToGroup(1, 2);
  gm.DisplayGroups();

  
  std::cout << " default active_group, need to set formally, " << unsigned(gm.GetActiveGroup()) << std::endl;
  DisplayTrackMuted(tm);

  // configure all set tracks to playback
  std::cout << "Set tracks to playback and set end index" << std::endl;
  uint32_t track_index = 0;
  for (auto &t : tm.tracks) {
    // To enter playback any track has to have an end_index > 0
    if (track_index < 9) {
      t.SetEndIndex(5);
    }
std::cout << " track " << track_index << " end_index " << t.GetEndIndex() << std::endl;
    t.SetTrackToInPlayback();
    track_index++;
  }
  DisplayTrackInPlayback(tm);

  // No tracks should be muted because default active group is 8 and there
  // have been no group assignments yet
  gm.DisplayGroups();
  DisplayTrackMuted(tm);

  std::cout << "Set active group for test 2" << std::endl;
  // Set Active group to 2, all but 5, 3, 1 should be muted
  // Simulate recording by setting end index
  tm.SetMasterEndIndex(5);
  // Simularte group 0 and 2's previous recording by setting it's end index copy
  gm.SetGroupMasterEndIndex(1, 0);
  gm.SetGroupMasterEndIndex(9, 2);
  gm.SetActiveGroup(2, tm);
  DisplayTrackMuted(tm);

  std::cout << "Set active group for test to 1" << std::endl;
  // Set Active group to 1, all but 2, 4, 6 should be muted
  // Simulate group 1 previoius recording by setting end index
  gm.SetGroupMasterEndIndex(5, 1);
  gm.SetActiveGroup(1, tm);
  DisplayTrackMuted(tm);

  std::cout << "Set active group for test to 2" << std::endl;
  gm.SetActiveGroup(2, tm);
  DisplayTrackMuted(tm);

  std::cout << "Set active group for test to 3" << std::endl;
  // no tracks in group 3 - all should be muted
  gm.SetActiveGroup(3, tm);
  DisplayTrackMuted(tm);

  std::cout << "Set active group for test 2" << std::endl;
  // Set Active group to 2, all but 5, 3, 1 should be muted
  gm.SetActiveGroup(2, tm);
  DisplayTrackMuted(tm);

  track_index = 0;
  for (auto &t : tm.tracks) {
    // To enter playback any track has to have an end_index > 0
std::cout << " track " << track_index << " end_index " << t.GetEndIndex() << std::endl;
    track_index++;
  }
  DisplayTrackInPlayback(tm);
  DisplayTrackMuted(tm);
}

void Test_NoTracksInGroups(GroupManager &gm, TrackManager &tm) {
  std::cout << std::endl << std::endl << "** test_group_manager.cpp: No Tracks In Groups **" << std::endl;
  // remove all tracks from all groups
  std::cout << "Removing tracks from groups.." << std::endl;
  gm.RemoveTrackFromGroup(5, 2);
  gm.RemoveTrackFromGroup(3, 2);
  gm.RemoveTrackFromGroup(1, 2);
  gm.RemoveTrackFromGroup(2, 1);
  gm.RemoveTrackFromGroup(4, 1);
  gm.RemoveTrackFromGroup(6, 1);
  gm.DisplayGroups();

  std::cout << "Setting all tracks to playback.." << std::endl;
  // set all tracks to playback
  for (auto &t : tm.tracks) {
    t.SetTrackToInPlayback();
  }
  std::cout << "Set active group to 0, should show all tracks muted, 1's " << std::endl;
  // set active group to 0
  gm.SetActiveGroup(0, tm);
  // all tracks should be muted
  DisplayTrackMuted(tm);
}

void Test_GroupSwapTests(GroupManager &gm, TrackManager &tm) {
  std::cout << std::endl << std::endl << "** test_group_manager.cpp: Group Swap Tests **" << std::endl;

  // empty all groups and end indexes
  std::cout << "    remove all tracks from all groups" << std::endl;
  for (uint8_t g = 0; g < MAX_GROUP_COUNT; g++) {
    for (uint32_t t = 0; t < MAX_TRACK_COUNT; t++) {
      gm.RemoveTrackFromGroup(t, g);
    }
    gm.SetGroupMasterEndIndex(0, g);
  }
  gm.ResetActiveGroupToNone();

  // set groups:
  // t0-3 - g0
  // t4-7 - g1
  // t8-11 - g2
  // rest should be zero
  std::cout << "    t0-3:g0, t4-7:g1, t8-11:g2" << std::endl;
  for (uint8_t g = 0; g < MAX_GROUP_COUNT; g++) {
    for (uint32_t t = 0; t < MAX_TRACK_COUNT; t++) {
      if (t < 4 && g == 0) {
        gm.AddTrackToGroup(t, g);
      } else if (t > 3 && t < 8 && g == 1) {
        gm.AddTrackToGroup(t, g);
      } else if (t > 7 && t < 12 && g == 2) {
        gm.AddTrackToGroup(t, g);
      } else {}
    }
  }

  gm.DisplayGroups();

  std::cout << "    set all track end indexes to 5" << std::endl;
  uint32_t track_index = 0;
  for (auto &t : tm.tracks) {
    t.SetEndIndex(5);
    track_index++;
  }

  std::cout << "    set first two tracks from each group to playback with repeat, rest play" << std::endl;
  tm.tracks.at(0).SetTrackToInPlaybackRepeat();
  tm.tracks.at(1).SetTrackToInPlaybackRepeat();
  tm.tracks.at(2).SetTrackToInPlayback();
  tm.tracks.at(3).SetTrackToInPlayback();
  tm.tracks.at(4).SetTrackToInPlaybackRepeat();
  tm.tracks.at(5).SetTrackToInPlaybackRepeat();
  tm.tracks.at(6).SetTrackToInPlayback();
  tm.tracks.at(7).SetTrackToInPlayback();
  tm.tracks.at(8).SetTrackToInPlaybackRepeat();
  tm.tracks.at(9).SetTrackToInPlaybackRepeat();
  tm.tracks.at(10).SetTrackToInPlayback();
  tm.tracks.at(11).SetTrackToInPlayback();
  tm.tracks.at(12).SetTrackToOff();
  tm.tracks.at(13).SetTrackToOff();
  tm.tracks.at(14).SetTrackToOff();
  tm.tracks.at(15).SetTrackToOff();

  // simulate previous recordings by setting group end indexes
  gm.SetGroupMasterEndIndex(5, 0);
  gm.SetGroupMasterEndIndex(7, 1);
  gm.SetGroupMasterEndIndex(9, 2);
  gm.SetGroupMasterEndIndex(3, 3);

  gm.DisplayGroups();
  DisplayTrackMuted(tm);

  std::cout << "Set active group for test 0" << std::endl;
  gm.SetActiveGroup(0, tm);
  DisplayTrackMuted(tm);

  std::cout << "Set active group for test 1" << std::endl;
  gm.SetActiveGroup(1, tm);
  DisplayTrackMuted(tm);

  std::cout << "Set active group for test 2" << std::endl;
  gm.SetActiveGroup(2, tm);
  DisplayTrackMuted(tm);

  std::cout << "Set active group for test 3" << std::endl;
  gm.SetActiveGroup(3, tm);
  DisplayTrackMuted(tm);

  std::cout << "Set active group for test 4" << std::endl;
  gm.SetActiveGroup(4, tm);
  DisplayTrackMuted(tm);

  std::cout << "Set active group for test 5" << std::endl;
  gm.SetActiveGroup(5, tm);
  DisplayTrackMuted(tm);

  std::cout << "Set active group for test 6" << std::endl;
  gm.SetActiveGroup(6, tm);
  DisplayTrackMuted(tm);

  std::cout << "Set active group for test 7" << std::endl;
  gm.SetActiveGroup(7, tm);
  DisplayTrackMuted(tm);

  std::cout << "Set active group for test 0" << std::endl;
  gm.SetActiveGroup(0, tm);
  DisplayTrackMuted(tm);

  std::cout << "Set active group for test 2" << std::endl;
  gm.SetActiveGroup(2, tm);
  DisplayTrackMuted(tm);

}


int main() {
  std::cout << "** test_group_manager.cpp **" << std::endl;
  GroupManager gm;

  Test_AddTracksToGroups(gm, tm);
  Test_RemoveTracksFromGroups(gm, tm);
  Test_ChangeActiveGroups(gm, tm);
  Test_NoTracksInGroups(gm, tm);
  Test_GroupSwapTests(gm, tm);
  return 0;
}
