// Loads UI assets by numeric layout ID. The switch contains 92 direct paths; 16 IDs use deprecated/fallback handling. SOMP IDs 101 and 102 fall through to preload subsequent layouts.
__int64 __fastcall ui_load_layout_by_id(__int64 a1, unsigned int a2, char a3)
{
  unsigned __int8 v5; // al
  int v6; // ecx
  __int64 v7; // rdi
  const char *v8; // rsi
  __int64 *v9; // r15
  __int64 v10; // rbx
  __int64 v11; // rsi
  int v12; // eax
  __int64 v13; // rbx
  __int64 v14; // rbx
  __int64 v16; // [rsp+0h] [rbp-310h] BYREF
  __int64 v17; // [rsp+8h] [rbp-308h] BYREF
  __int64 v18; // [rsp+10h] [rbp-300h] BYREF
  __int64 v19; // [rsp+18h] [rbp-2F8h] BYREF
  __int64 v20; // [rsp+20h] [rbp-2F0h] BYREF
  __int64 v21; // [rsp+28h] [rbp-2E8h] BYREF
  __int64 v22; // [rsp+30h] [rbp-2E0h] BYREF
  __int64 v23; // [rsp+38h] [rbp-2D8h] BYREF
  __int64 v24; // [rsp+40h] [rbp-2D0h] BYREF
  __int64 v25; // [rsp+48h] [rbp-2C8h] BYREF
  __int64 v26; // [rsp+50h] [rbp-2C0h] BYREF
  __int64 v27; // [rsp+58h] [rbp-2B8h] BYREF
  __int64 v28; // [rsp+60h] [rbp-2B0h] BYREF
  __int64 v29; // [rsp+68h] [rbp-2A8h] BYREF
  __int64 v30; // [rsp+70h] [rbp-2A0h] BYREF
  __int64 v31; // [rsp+78h] [rbp-298h] BYREF
  __int64 v32; // [rsp+80h] [rbp-290h] BYREF
  __int64 v33; // [rsp+88h] [rbp-288h] BYREF
  __int64 v34; // [rsp+90h] [rbp-280h] BYREF
  __int64 v35; // [rsp+98h] [rbp-278h] BYREF
  __int64 v36; // [rsp+A0h] [rbp-270h] BYREF
  __int64 v37; // [rsp+A8h] [rbp-268h] BYREF
  __int64 v38; // [rsp+B0h] [rbp-260h] BYREF
  __int64 v39; // [rsp+B8h] [rbp-258h] BYREF
  __int64 v40; // [rsp+C0h] [rbp-250h] BYREF
  __int64 v41; // [rsp+C8h] [rbp-248h] BYREF
  __int64 v42; // [rsp+D0h] [rbp-240h] BYREF
  __int64 v43; // [rsp+D8h] [rbp-238h] BYREF
  __int64 v44; // [rsp+E0h] [rbp-230h] BYREF
  __int64 v45; // [rsp+E8h] [rbp-228h] BYREF
  __int64 v46; // [rsp+F0h] [rbp-220h] BYREF
  __int64 v47; // [rsp+F8h] [rbp-218h] BYREF
  __int64 v48; // [rsp+100h] [rbp-210h] BYREF
  __int64 v49; // [rsp+108h] [rbp-208h] BYREF
  __int64 v50; // [rsp+110h] [rbp-200h] BYREF
  __int64 v51; // [rsp+118h] [rbp-1F8h] BYREF
  __int64 v52; // [rsp+120h] [rbp-1F0h] BYREF
  __int64 v53; // [rsp+128h] [rbp-1E8h] BYREF
  __int64 v54; // [rsp+130h] [rbp-1E0h] BYREF
  __int64 v55; // [rsp+138h] [rbp-1D8h] BYREF
  __int64 v56; // [rsp+140h] [rbp-1D0h] BYREF
  __int64 v57; // [rsp+148h] [rbp-1C8h] BYREF
  __int64 v58; // [rsp+150h] [rbp-1C0h] BYREF
  __int64 v59; // [rsp+158h] [rbp-1B8h] BYREF
  __int64 v60; // [rsp+160h] [rbp-1B0h] BYREF
  __int64 v61; // [rsp+168h] [rbp-1A8h] BYREF
  __int64 v62; // [rsp+170h] [rbp-1A0h] BYREF
  __int64 v63; // [rsp+178h] [rbp-198h] BYREF
  __int64 v64; // [rsp+180h] [rbp-190h] BYREF
  __int64 v65; // [rsp+188h] [rbp-188h] BYREF
  __int64 v66; // [rsp+190h] [rbp-180h] BYREF
  __int64 v67; // [rsp+198h] [rbp-178h] BYREF
  __int64 v68; // [rsp+1A0h] [rbp-170h] BYREF
  __int64 v69; // [rsp+1A8h] [rbp-168h] BYREF
  __int64 v70; // [rsp+1B0h] [rbp-160h] BYREF
  __int64 v71; // [rsp+1B8h] [rbp-158h] BYREF
  __int64 v72; // [rsp+1C0h] [rbp-150h] BYREF
  __int64 v73; // [rsp+1C8h] [rbp-148h] BYREF
  __int64 v74; // [rsp+1D0h] [rbp-140h] BYREF
  __int64 v75; // [rsp+1D8h] [rbp-138h] BYREF
  __int64 v76; // [rsp+1E0h] [rbp-130h] BYREF
  __int64 v77; // [rsp+1E8h] [rbp-128h] BYREF
  __int64 v78; // [rsp+1F0h] [rbp-120h] BYREF
  __int64 v79; // [rsp+1F8h] [rbp-118h] BYREF
  __int64 v80; // [rsp+200h] [rbp-110h] BYREF
  __int64 v81; // [rsp+208h] [rbp-108h] BYREF
  __int64 v82; // [rsp+210h] [rbp-100h] BYREF
  __int64 v83; // [rsp+218h] [rbp-F8h] BYREF
  __int64 v84; // [rsp+220h] [rbp-F0h] BYREF
  __int64 v85; // [rsp+228h] [rbp-E8h] BYREF
  __int64 v86; // [rsp+230h] [rbp-E0h] BYREF
  __int64 v87; // [rsp+238h] [rbp-D8h] BYREF
  __int64 v88; // [rsp+240h] [rbp-D0h] BYREF
  __int64 v89; // [rsp+248h] [rbp-C8h] BYREF
  __int64 v90; // [rsp+250h] [rbp-C0h] BYREF
  __int64 v91; // [rsp+258h] [rbp-B8h] BYREF
  __int64 v92; // [rsp+260h] [rbp-B0h] BYREF
  __int64 v93; // [rsp+268h] [rbp-A8h] BYREF
  __int64 v94; // [rsp+270h] [rbp-A0h] BYREF
  __int64 v95; // [rsp+278h] [rbp-98h] BYREF
  __int64 v96; // [rsp+280h] [rbp-90h] BYREF
  __int64 v97; // [rsp+288h] [rbp-88h] BYREF
  __int64 v98; // [rsp+290h] [rbp-80h] BYREF
  __int64 v99; // [rsp+298h] [rbp-78h] BYREF
  __int64 v100; // [rsp+2A0h] [rbp-70h] BYREF
  __int64 v101; // [rsp+2A8h] [rbp-68h] BYREF
  __int64 v102; // [rsp+2B0h] [rbp-60h] BYREF
  __int64 v103; // [rsp+2B8h] [rbp-58h] BYREF
  __int64 v104; // [rsp+2C0h] [rbp-50h] BYREF
  __int64 v105; // [rsp+2C8h] [rbp-48h] BYREF
  __int64 v106; // [rsp+2D0h] [rbp-40h] BYREF
  _QWORD v107[7]; // [rsp+2D8h] [rbp-38h] BYREF

  v107[1] = 0x6365786562696C2FLL;
  if ( unk_1B1E030 != 0 && a3 == 0 )
    ui_layout_id_to_symbol(a2);
  if ( *(_DWORD *)(a1 + 24) != a2 )
  {
    v5 = 1;
    *(_DWORD *)(a1 + 28) = a2;
    if ( a2 > 9 || (v6 = 524, !_bittest(&v6, a2)) )
    {
      if ( a2 != 92 )
        v5 = 0;
    }
    ui_adjust_loading_thread_priorities(a1, v5);
    switch ( a2 )
    {
      case 0u:
        v8 = "ui/title/title.layout";
        v9 = v107;
        v10 = unk_1AC9A78;
        v107[0] = 19246190;
        goto LABEL_108;
      case 1u:
        v8 = "ui/songs/songs.layout";
        v9 = &v106;
        v10 = unk_1AC9A78;
        v106 = 19246190;
        goto LABEL_108;
      case 2u:
        v8 = "ui/difficulty/difficulty.layout";
        v9 = &v105;
        v10 = unk_1AC9A78;
        v105 = 19246190;
        goto LABEL_108;
      case 3u:
        v8 = "ui/game/game.layout";
        v9 = &v104;
        v10 = unk_1AC9A78;
        v104 = 19246190;
        goto LABEL_108;
      case 4u:
        v8 = "ui/main_menu/main_menu.layout";
        v9 = &v103;
        v10 = unk_1AC9A78;
        v103 = 19246190;
        goto LABEL_108;
      case 5u:
      case 0x11u:
      case 0x18u:
      case 0x25u:
      case 0x32u:
      case 0x34u:
      case 0x3Eu:
      case 0x3Fu:
      case 0x49u:
      case 0x57u:
        break;
      case 6u:
        v8 = "ui/store/store_categories.layout";
        v9 = &v97;
        v10 = unk_1AC9A78;
        v97 = 19246190;
        goto LABEL_108;
      case 7u:
        v8 = "ui/store/store_songs.layout";
        v9 = &v96;
        v10 = unk_1AC9A78;
        v96 = 19246190;
        goto LABEL_108;
      case 8u:
        v8 = "ui/results/campaign_gig_complete_results.layout";
        v9 = &v94;
        v10 = unk_1AC9A78;
        v94 = 19246190;
        goto LABEL_108;
      case 9u:
        v8 = "ui/loading/loading.layout";
        v9 = &v93;
        v10 = unk_1AC9A78;
        v93 = 19246190;
        goto LABEL_108;
      case 0xAu:
        v8 = "ui/band_edit/band_edit.layout";
        v9 = &v92;
        v10 = unk_1AC9A78;
        v92 = 19246190;
        goto LABEL_108;
      case 0xBu:
        v8 = "ui/store/store_search.layout";
        v9 = &v95;
        v10 = unk_1AC9A78;
        v95 = 19246190;
        goto LABEL_108;
      case 0xCu:
        v8 = "ui/band_select/band_select.layout";
        v9 = &v91;
        v10 = unk_1AC9A78;
        v91 = 19246190;
        goto LABEL_108;
      case 0xDu:
        v8 = "ui/city_select/city_select.layout";
        v9 = &v90;
        v10 = unk_1AC9A78;
        v90 = 19246190;
        goto LABEL_108;
      case 0xEu:
        v8 = "ui/leaderboards/leaderboards.layout";
        v9 = &v89;
        v10 = unk_1AC9A78;
        v89 = 19246190;
        goto LABEL_108;
      case 0xFu:
        v8 = "ui/player_stats/player_stats.layout";
        v9 = &v88;
        v10 = unk_1AC9A78;
        v88 = 19246190;
        goto LABEL_108;
      case 0x10u:
        v8 = "ui/campaign_narrative/campaign_narrative.layout";
        v9 = &v87;
        v10 = unk_1AC9A78;
        v87 = 19246190;
        goto LABEL_108;
      case 0x12u:
        v8 = "ui/store/store_home.layout";
        v9 = &v86;
        v10 = unk_1AC9A78;
        v86 = 19246190;
        goto LABEL_108;
      case 0x13u:
        v8 = "ui/campaign_gig_select/campaign_gig_select.layout";
        v9 = &v85;
        v10 = unk_1AC9A78;
        v85 = 19246190;
        goto LABEL_108;
      case 0x14u:
        v8 = "ui/tour_select/tour_select.layout";
        v9 = &v84;
        v10 = unk_1AC9A78;
        v84 = 19246190;
        goto LABEL_108;
      case 0x15u:
        v8 = "ui/options/options_system_settings.layout";
        v9 = &v102;
        v10 = unk_1AC9A78;
        v102 = 19246190;
        goto LABEL_108;
      case 0x16u:
        v8 = "ui/options/options_volume_controls.layout";
        v9 = &v101;
        v10 = unk_1AC9A78;
        v101 = 19246190;
        goto LABEL_108;
      case 0x17u:
        v8 = "ui/band_naming/band_naming.layout";
        v9 = &v83;
        v10 = unk_1AC9A78;
        v83 = 19246190;
        goto LABEL_108;
      case 0x19u:
        v8 = "ui/options/options_credits.layout";
        v9 = &v100;
        v10 = unk_1AC9A78;
        v100 = 19246190;
        goto LABEL_108;
      case 0x1Au:
        v8 = "ui/options/options_modifiers.layout";
        v9 = &v99;
        v10 = unk_1AC9A78;
        v99 = 19246190;
        goto LABEL_108;
      case 0x1Bu:
        v8 = "ui/legacy_entitlements/legacy_entitlements.layout";
        v9 = &v82;
        v10 = unk_1AC9A78;
        v82 = 19246190;
        goto LABEL_108;
      case 0x1Eu:
        v8 = "ui/character/character_create/character_create.layout";
        v9 = &v80;
        v10 = unk_1AC9A78;
        v80 = 19246190;
        goto LABEL_108;
      case 0x20u:
        v8 = "ui/character/character_customize/character_customize.layout";
        v9 = &v77;
        v10 = unk_1AC9A78;
        v77 = 19246190;
        goto LABEL_108;
      case 0x22u:
        v8 = "ui/results/quickplay_results.layout";
        v9 = &v81;
        v10 = unk_1AC9A78;
        v81 = 19246190;
        goto LABEL_108;
      case 0x23u:
        v8 = "ui/character/character_create/character_create_head.layout";
        v9 = &v79;
        v10 = unk_1AC9A78;
        v79 = 19246190;
        goto LABEL_108;
      case 0x24u:
        v8 = "ui/character/character_create/character_create_hair.layout";
        v9 = &v78;
        v10 = unk_1AC9A78;
        v78 = 19246190;
        goto LABEL_108;
      case 0x26u:
        v8 = "ui/character/character_customize/character_customize_clothing.layout";
        v9 = &v76;
        v10 = unk_1AC9A78;
        v76 = 19246190;
        goto LABEL_108;
      case 0x27u:
        v8 = "ui/character/character_customize/character_customize_hair_makeup.layout";
        v9 = &v75;
        v10 = unk_1AC9A78;
        v75 = 19246190;
        goto LABEL_108;
      case 0x28u:
        v8 = "ui/character/character_customize/character_customize_instruments.layout";
        v9 = &v74;
        v10 = unk_1AC9A78;
        v74 = 19246190;
        goto LABEL_108;
      case 0x29u:
        v8 = "ui/character/character_customize/character_customize_detail.layout";
        v9 = &v73;
        v10 = unk_1AC9A78;
        v73 = 19246190;
        goto LABEL_108;
      case 0x2Au:
        v8 = "ui/band_hub/band_hub.layout";
        v9 = &v69;
        v10 = unk_1AC9A78;
        v69 = 19246190;
        goto LABEL_108;
      case 0x2Bu:
        v8 = "ui/band_member_select/band_member_select.layout";
        v9 = &v68;
        v10 = unk_1AC9A78;
        v68 = 19246190;
        goto LABEL_108;
      case 0x2Cu:
        v8 = "ui/startup/startup.layout";
        v9 = &v67;
        v10 = unk_1AC9A78;
        v67 = 19246190;
        goto LABEL_108;
      case 0x2Du:
        v8 = "ui/calibration/calibration_auto.layout";
        v9 = &v66;
        v10 = unk_1AC9A78;
        v66 = 19246190;
        goto LABEL_108;
      case 0x2Eu:
        v8 = "ui/calibration/calibration_manual.layout";
        v9 = &v65;
        v10 = unk_1AC9A78;
        v65 = 19246190;
        goto LABEL_108;
      case 0x2Fu:
        v8 = "ui/calibration/calibration_enter_numbers.layout";
        v9 = &v64;
        v10 = unk_1AC9A78;
        v64 = 19246190;
        goto LABEL_108;
      case 0x30u:
        v8 = "ui/character/character_customize/character_customize_band_select.layout";
        v9 = &v72;
        v10 = unk_1AC9A78;
        v72 = 19246190;
        goto LABEL_108;
      case 0x31u:
        v8 = "ui/band_member_select/session_member_select.layout";
        v9 = &v63;
        v10 = unk_1AC9A78;
        v63 = 19246190;
        goto LABEL_108;
      case 0x33u:
        v8 = "ui/campaign_setlist/setlist.layout";
        v9 = &v62;
        v10 = unk_1AC9A78;
        v62 = 19246190;
        goto LABEL_108;
      case 0x35u:
        v8 = "ui/songs/songs_search.layout";
        v9 = &v60;
        v10 = unk_1AC9A78;
        v60 = 19246190;
        goto LABEL_108;
      case 0x36u:
        v8 = "ui/guitar_solo_invite/guitar_solo_invite.layout";
        v9 = &v59;
        v10 = unk_1AC9A78;
        v59 = 19246190;
        goto LABEL_108;
      case 0x37u:
        v8 = "ui/band_history/band_history.layout";
        v9 = &v58;
        v10 = unk_1AC9A78;
        v58 = 19246190;
        goto LABEL_108;
      case 0x38u:
        v8 = "ui/notifications/notifications.layout";
        v9 = &v57;
        v10 = unk_1AC9A78;
        v57 = 19246190;
        goto LABEL_108;
      case 0x39u:
        v8 = "ui/user_setlist/setlist_hub.layout";
        v9 = &v56;
        v10 = unk_1AC9A78;
        v56 = 19246190;
        goto LABEL_108;
      case 0x3Au:
        v8 = "ui/user_setlist_naming/setlist_naming.layout";
        v9 = &v55;
        v10 = unk_1AC9A78;
        v55 = 19246190;
        goto LABEL_108;
      case 0x3Bu:
        v12 = *(_DWORD *)(a1 + 24);
        if ( v12 != 3 && v12 != 60 )
          *(_DWORD *)(a1 + 336) = 0;
        v8 = "ui/practice/practice_section_select.layout";
        v9 = &v54;
        v10 = unk_1AC9A78;
        v54 = 19246190;
        goto LABEL_108;
      case 0x3Cu:
        v8 = "ui/practice/practice_speed_select.layout";
        v9 = &v53;
        v10 = unk_1AC9A78;
        v53 = 19246190;
        goto LABEL_108;
      case 0x3Du:
        v8 = "ui/btm/btm_setlist.layout";
        v9 = &v61;
        v10 = unk_1AC9A78;
        v61 = 19246190;
        goto LABEL_108;
      case 0x40u:
        v8 = "ui/band_wars/clan_rivals_hub.layout";
        v9 = &v52;
        v10 = unk_1AC9A78;
        v52 = 19246190;
        goto LABEL_108;
      case 0x41u:
        v8 = "ui/player_stats/player_v_player.layout";
        v9 = &v51;
        v10 = unk_1AC9A78;
        v51 = 19246190;
        goto LABEL_108;
      case 0x42u:
        v8 = "ui/player_stats/clan_v_clan.layout";
        v9 = &v50;
        v10 = unk_1AC9A78;
        v50 = 19246190;
        goto LABEL_108;
      case 0x43u:
        v8 = "ui/band_wars/find_players.layout";
        v9 = &v49;
        v10 = unk_1AC9A78;
        v49 = 19246190;
        goto LABEL_108;
      case 0x44u:
        v8 = "ui/band_wars/my_invitations.layout";
        v9 = &v48;
        v10 = unk_1AC9A78;
        v48 = 19246190;
        goto LABEL_108;
      case 0x45u:
        v11 = 69;
        goto LABEL_72;
      case 0x46u:
        v8 = "ui/btm/btm_hub.layout";
        v9 = &v47;
        v10 = unk_1AC9A78;
        v47 = 19246190;
        goto LABEL_108;
      case 0x47u:
        v8 = "ui/btm/btm_band_select.layout";
        v9 = &v46;
        v10 = unk_1AC9A78;
        v46 = 19246190;
        goto LABEL_108;
      case 0x48u:
        sub_BB6FC0(v7, 72);
        v8 = "ui/band_wars/clan_badges.layout";
        v9 = &v45;
        v10 = unk_1AC9A78;
        v45 = 19246190;
        goto LABEL_108;
      case 0x4Au:
        v8 = "ui/btm/btm_intro_choice.layout";
        v9 = &v44;
        v10 = unk_1AC9A78;
        v44 = 19246190;
        goto LABEL_108;
      case 0x4Bu:
        v8 = "ui/btm/btm_win_loss.layout";
        v9 = &v43;
        v10 = unk_1AC9A78;
        v43 = 19246190;
        goto LABEL_108;
      case 0x4Cu:
        v8 = "ui/btm/btm_chapter_completion.layout";
        v9 = &v42;
        v10 = unk_1AC9A78;
        v42 = 19246190;
        goto LABEL_108;
      case 0x4Du:
        v8 = "ui/band_wars/find_groups.layout";
        v9 = &v41;
        v10 = unk_1AC9A78;
        v41 = 19246190;
        goto LABEL_108;
      case 0x4Eu:
        v8 = "ui/band_naming/btm_band_naming.layout";
        v9 = &v40;
        v10 = unk_1AC9A78;
        v40 = 19246190;
        goto LABEL_108;
      case 0x4Fu:
        v8 = "ui/band_wars/clan_notifications.layout";
        v9 = &v39;
        v10 = unk_1AC9A78;
        v39 = 19246190;
        goto LABEL_108;
      case 0x50u:
        v8 = "ui/band_wars_clan_stats/clan_stats.layout";
        v9 = &v38;
        v10 = unk_1AC9A78;
        v38 = 19246190;
        goto LABEL_108;
      case 0x51u:
        v8 = "ui/band_wars/clan_badges.layout";
        v9 = &v37;
        v10 = unk_1AC9A78;
        v37 = 19246190;
        goto LABEL_108;
      case 0x52u:
        v8 = "ui/player_stats/player_badges.layout";
        v9 = &v36;
        v10 = unk_1AC9A78;
        v36 = 19246190;
        goto LABEL_108;
      case 0x53u:
        v8 = "ui/portrait/player_portrait.layout";
        v9 = &v35;
        v10 = unk_1AC9A78;
        v35 = 19246190;
        goto LABEL_108;
      case 0x54u:
        v8 = "ui/band_wars/clan_create_edit.layout";
        v9 = &v34;
        v10 = unk_1AC9A78;
        v34 = 19246190;
        goto LABEL_108;
      case 0x56u:
        v8 = "ui/btm/btm_mode_completion.layout";
        v9 = &v30;
        v10 = unk_1AC9A78;
        v30 = 19246190;
        goto LABEL_108;
      case 0x58u:
        v8 = "ui/band_wars/clan_roster.layout";
        v9 = &v29;
        v10 = unk_1AC9A78;
        v29 = 19246190;
        goto LABEL_108;
      case 0x59u:
        v8 = "ui/band_wars/band_wars_weekly_challenges.layout";
        v9 = &v27;
        v10 = unk_1AC9A78;
        v27 = 19246190;
        goto LABEL_108;
      case 0x5Au:
        v8 = "ui/band_wars/clan_tier_status.layout";
        v9 = &v26;
        v10 = unk_1AC9A78;
        v26 = 19246190;
        goto LABEL_108;
      case 0x5Bu:
        v8 = "ui/band_wars/clan_rivals_event_leaderboard.layout";
        v9 = &v25;
        v10 = unk_1AC9A78;
        v25 = 19246190;
        goto LABEL_108;
      case 0x5Cu:
        v8 = "ui/btm/btm_challenge.layout";
        v9 = &v24;
        v10 = unk_1AC9A78;
        v24 = 19246190;
        goto LABEL_108;
      case 0x5Du:
        v8 = "ui/somp/somp_green_room/somp_green_room.layout";
        v9 = &v23;
        v10 = unk_1AC9A78;
        v23 = 19246190;
        goto LABEL_108;
      case 0x5Eu:
        v8 = "ui/somp/somp_menu.layout";
        v9 = &v22;
        v10 = unk_1AC9A78;
        v22 = 19246190;
        goto LABEL_108;
      case 0x5Fu:
        v8 = "ui/somp/somp_hub/somp_hub.layout";
        v9 = &v21;
        v10 = unk_1AC9A78;
        v21 = 19246190;
        goto LABEL_108;
      case 0x60u:
        v8 = "ui/somp/somp_invite_friends/somp_invite_friends.layout";
        v9 = &v20;
        v10 = unk_1AC9A78;
        v20 = 19246190;
        goto LABEL_108;
      case 0x61u:
        v8 = "ui/missions/missions.layout";
        v9 = &v19;
        v10 = unk_1AC9A78;
        v19 = 19246190;
        goto LABEL_108;
      case 0x62u:
        v8 = "ui/band_wars/band_wars_user_settings.layout";
        v9 = &v33;
        v10 = unk_1AC9A78;
        v33 = 19246190;
        goto LABEL_108;
      case 0x63u:
        v8 = "ui/band_wars/clan_rivals_looking_for_clan.layout";
        v9 = &v32;
        v10 = unk_1AC9A78;
        v32 = 19246190;
        goto LABEL_108;
      case 0x64u:
        v8 = "ui/band_wars/clan_rivals_invitations.layout";
        v9 = &v31;
        v10 = unk_1AC9A78;
        v31 = 19246190;
        goto LABEL_108;
      case 0x65u:
        v13 = unk_1AC9A78;
        v18 = 19246190;
        sub_1AF950(&v18, "ui/somp/somp_session_create_edit/somp_session_create_edit.layout");
        sub_8C9F50(v13, &v18);
        goto LABEL_102;
      case 0x66u:
LABEL_102:
        v17 = 19246190;
        v14 = unk_1AC9A78;
        sub_1AF950(&v17, "ui/somp/somp_browse_sessions/somp_browse_sessions.layout");
        sub_8C9F50(v14, &v17);
        goto LABEL_103;
      case 0x67u:
LABEL_103:
        v8 = "ui/somp/somp_session_preferences/somp_session_preferences.layout";
        v9 = &v16;
        v16 = 19246190;
        v10 = unk_1AC9A78;
        goto LABEL_108;
      case 0x68u:
        v8 = "ui/options/options_graphics.layout";
        v9 = &v98;
        v10 = unk_1AC9A78;
        v98 = 19246190;
        goto LABEL_108;
      case 0x69u:
        v8 = "ui/band_wars/clan_info_landing.layout";
        v9 = &v28;
        v10 = unk_1AC9A78;
        v28 = 19246190;
        goto LABEL_108;
      case 0x6Au:
        v8 = "ui/character/character_customize/character_customize_track.layout";
        v9 = &v71;
        v10 = unk_1AC9A78;
        v71 = 19246190;
        goto LABEL_108;
      case 0x6Bu:
        v8 = "ui/character/character_customize/character_customize_track_r2t.layout";
        v9 = &v70;
        v10 = unk_1AC9A78;
        v70 = 19246190;
LABEL_108:
        sub_1AF950(v9, v8);
        sub_8C9F50(v10, v9);
        break;
      default:
        v11 = a2;
LABEL_72:
        sub_BB6FC0(v7, v11);
        ui_load_layout_by_id(a1, 0, 0);
        break;
    }
    *(_BYTE *)(a1 + 321) = 1;
    sub_ADDB80(v7);
  }
  return 0x6365786562696C2FLL;
}
