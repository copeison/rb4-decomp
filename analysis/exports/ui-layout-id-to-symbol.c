// Lazily interns the 108 UiLayoutId names. ID -1 returns kLayoutInvalid; IDs 0 through 107 index the guarded symbol table.
__int64 ui_layout_id_to_symbol(int id)
{
  __int64 result; // rax
  __int64 v3; // [rsp+0h] [rbp-20h] BYREF
  __int64 v4; // [rsp+8h] [rbp-18h]

  v4 = 0x6365786562696C2FLL;
  if ( id == -1 )
  {
    sub_256FD0(&v3, "kLayoutInvalid");
    result = v3;
  }
  else
  {
    if ( byte_1AFF510 == 0 && (unsigned int)sub_1243270(&byte_1AFF510) != 0 )
    {
      sub_256FD0(qword_1AFF1B0, "kLayoutTitle");
      sub_256FD0(&unk_1AFF1B8, "kLayoutSongSelect");
      sub_256FD0(&unk_1AFF1C0, "kLayoutDifficultySelect");
      sub_256FD0(&unk_1AFF1C8, "kLayoutGame");
      sub_256FD0(&unk_1AFF1D0, "kLayoutMainMenu");
      sub_256FD0(&unk_1AFF1D8, "kLayoutOptions_DEPRECATED");
      sub_256FD0(&unk_1AFF1E0, "kLayoutStoreCategories");
      sub_256FD0(&unk_1AFF1E8, "kLayoutStoreSongs");
      sub_256FD0(&unk_1AFF1F0, "kLayoutCampaignGigCompletionResults");
      sub_256FD0(&unk_1AFF1F8, "kLayoutLoading");
      sub_256FD0(&unk_1AFF200, "kLayoutBandEdit");
      sub_256FD0(&unk_1AFF208, "kLayoutStoreSearch");
      sub_256FD0(&unk_1AFF210, "kLayoutBandSelection");
      sub_256FD0(&unk_1AFF218, "kLayoutCitySelection");
      sub_256FD0(&unk_1AFF220, "kLayoutLeaderboards");
      sub_256FD0(&unk_1AFF228, "kLayoutPlayerStats");
      sub_256FD0(&unk_1AFF230, "kLayoutCampaignNarrativeUpdate");
      sub_256FD0(&unk_1AFF238, "kLayoutVenueSelect_DEPRECATED");
      sub_256FD0(&unk_1AFF240, "kLayoutStoreHome");
      sub_256FD0(&unk_1AFF248, "kLayoutGigSelection");
      sub_256FD0(&unk_1AFF250, "kLayoutTourSelection");
      sub_256FD0(&unk_1AFF258, "kLayoutOptionsSystemSettings");
      sub_256FD0(&unk_1AFF260, "kLayoutOptionsVolumeControls");
      sub_256FD0(&unk_1AFF268, "kLayoutBandNaming");
      sub_256FD0(&unk_1AFF270, "kLayoutOptionsCalibration_DEPRECATED");
      sub_256FD0(&unk_1AFF278, "kLayoutOptionsCredits");
      sub_256FD0(&unk_1AFF280, "kLayoutOptionsModifiers");
      sub_256FD0(&unk_1AFF288, "kLayoutLegacyEntitlements_DEPRECATED");
      sub_256FD0(&unk_1AFF290, "kLayoutCharacterCreator_DEPRECATED");
      sub_256FD0(&unk_1AFF298, "kLayoutCharacterMain_DEPRECATED");
      sub_256FD0(&unk_1AFF2A0, "kLayoutCharacterCreate");
      sub_256FD0(&unk_1AFF2A8, "kLayoutCharacterSelect_DEPRECATED");
      sub_256FD0(&unk_1AFF2B0, "kLayoutRockShopMain");
      sub_256FD0(&unk_1AFF2B8, "kLayoutCharacterDelete_DEPRECATED");
      sub_256FD0(&unk_1AFF2C0, "kLayoutQuickplayResults");
      sub_256FD0(&unk_1AFF2C8, "kLayoutCharacterCreateHead");
      sub_256FD0(&unk_1AFF2D0, "kLayoutCharacterCreateHair");
      sub_256FD0(&unk_1AFF2D8, "kLayoutCharacterCreateFacialHair_DEPRECATED");
      sub_256FD0(&unk_1AFF2E0, "kLayoutRockShopClothing");
      sub_256FD0(&unk_1AFF2E8, "kLayoutRockShopHairMakeup");
      sub_256FD0(&unk_1AFF2F0, "kLayoutRockShopInstruments");
      sub_256FD0(&unk_1AFF2F8, "kLayoutRockShopDetail");
      sub_256FD0(&unk_1AFF300, "kLayoutBandHub");
      sub_256FD0(&unk_1AFF308, "kLayoutBandMemberSelect");
      sub_256FD0(&unk_1AFF310, "kLayoutGameStartup");
      sub_256FD0(&unk_1AFF318, "kLayoutCalibrationAuto");
      sub_256FD0(&unk_1AFF320, "kLayoutCalibrationManual");
      sub_256FD0(&unk_1AFF328, "kLayoutCalibrationEnterNumbers");
      sub_256FD0(&unk_1AFF330, "kLayoutRockShopBandSelect");
      sub_256FD0(&unk_1AFF338, "kLayoutSessionMusicianSelect");
      sub_256FD0(&unk_1AFF340, "kLayoutIntro_DEPRECATED");
      sub_256FD0(&unk_1AFF348, "kLayoutCampaignSetlistCreate");
      sub_256FD0(&unk_1AFF350, "kLayoutGuitarSoloExtras_DEPRECATED");
      sub_256FD0(&unk_1AFF358, "kLayoutSongSelectSearch");
      sub_256FD0(&unk_1AFF360, "kLayoutGuitarSoloTutorialInvite");
      sub_256FD0(&unk_1AFF368, "kLayoutCampaignBandHistory");
      sub_256FD0(&unk_1AFF370, "kLayoutNotificationFeed");
      sub_256FD0(&unk_1AFF378, "kLayoutSetlistHub");
      sub_256FD0(&unk_1AFF380, "kLayoutSetlistNaming");
      sub_256FD0(&unk_1AFF388, "kLayoutPracticeSectionSelect");
      sub_256FD0(&unk_1AFF390, "kLayoutPracticeSpeedSelect");
      sub_256FD0(&unk_1AFF398, "kLayoutBTMSetlistCreate");
      sub_256FD0(&unk_1AFF3A0, "kLayoutPlayerProfileStats_DEPRECATED");
      sub_256FD0(&unk_1AFF3A8, "kLayoutClanProfileStats_DEPRECATED");
      sub_256FD0(&unk_1AFF3B0, "kLayoutBandWarsHub");
      sub_256FD0(&unk_1AFF3B8, "kLayoutPlayerVsPlayer_DEPRECATED");
      sub_256FD0(&unk_1AFF3C0, "kLayoutClanVsClan");
      sub_256FD0(&unk_1AFF3C8, "kLayoutFindPlayers");
      sub_256FD0(&unk_1AFF3D0, "kLayoutMyInvitations");
      sub_256FD0(&unk_1AFF3D8, "kLayoutDevMenu");
      sub_256FD0(&unk_1AFF3E0, "kLayoutBTMHub");
      sub_256FD0(&unk_1AFF3E8, "kLayoutBTMBandSelection");
      sub_256FD0(&unk_1AFF3F0, "kLayoutClanAchievements_DEPRECATED");
      sub_256FD0(&unk_1AFF3F8, "kLayoutBTMIntro_DEPRECATED");
      sub_256FD0(&unk_1AFF400, "kLayoutBTMIntroChoice");
      sub_256FD0(&unk_1AFF408, "kLayoutBTMWinLoss");
      sub_256FD0(&unk_1AFF410, "kLayoutBTMChapterCompletion");
      sub_256FD0(&unk_1AFF418, "kLayoutFindClans");
      sub_256FD0(&unk_1AFF420, "kLayoutBTMBandNaming");
      sub_256FD0(&unk_1AFF428, "kLayoutClanNotifications");
      sub_256FD0(&unk_1AFF430, "kLayoutClanStats");
      sub_256FD0(&unk_1AFF438, "kLayoutClanBadges");
      sub_256FD0(&unk_1AFF440, "kLayoutPlayerBadges");
      sub_256FD0(&unk_1AFF448, "kLayoutPlayerPortrait");
      sub_256FD0(&unk_1AFF450, "kLayoutClanCreateEdit");
      sub_256FD0(&unk_1AFF458, "kLayoutBandWarsHubNoClan_DEPRECATED");
      sub_256FD0(&unk_1AFF460, "kLayoutBTMModeCompletion");
      sub_256FD0(&unk_1AFF468, "kLayoutClanWeeklyChallengeResult_DEPRECATED");
      sub_256FD0(&unk_1AFF470, "kLayoutClanRoster");
      sub_256FD0(&unk_1AFF478, "kLayoutClanWeeklyChallenge");
      sub_256FD0(&unk_1AFF480, "kLayoutClanTierStatus");
      sub_256FD0(&unk_1AFF488, "kLayoutClanEventLeaderboard");
      sub_256FD0(&unk_1AFF490, "kLayoutBTMChallenge");
      sub_256FD0(&unk_1AFF498, "kLayoutSompGreenRoom");
      sub_256FD0(&unk_1AFF4A0, "kLayoutSompMenu");
      sub_256FD0(&unk_1AFF4A8, "kLayoutSompHub");
      sub_256FD0(&unk_1AFF4B0, "kLayoutSompInviteFriends");
      sub_256FD0(&unk_1AFF4B8, "kLayoutMissions");
      sub_256FD0(&unk_1AFF4C0, "kLayoutClanUserSettings");
      sub_256FD0(&unk_1AFF4C8, "kLayoutBandWarsLookingForClan");
      sub_256FD0(&unk_1AFF4D0, "kLayoutBandWarsLookingForClanInvitations");
      sub_256FD0(&unk_1AFF4D8, "kLayoutSompSessionCreateEdit");
      sub_256FD0(&unk_1AFF4E0, "kLayoutSompBrowseSessions");
      sub_256FD0(&unk_1AFF4E8, "kLayoutSompSessionPreferences");
      sub_256FD0(&unk_1AFF4F0, "kLayoutOptionsGraphics");
      sub_256FD0(&unk_1AFF4F8, "kLayoutClanInfoLanding");
      sub_256FD0(&unk_1AFF500, "kLayoutRockShopCustomizeTrack");
      sub_256FD0(&unk_1AFF508, "kLayoutRockShopCustomizeTrackR2T");
      sub_1243280(&byte_1AFF510);
    }
    result = qword_1AFF1B0[id];
    v3 = result;
  }
  if ( v4 != 0x6365786562696C2FLL )
  {
    sub_1243210();
    return sub_BB0F40();
  }
  return result;
}
