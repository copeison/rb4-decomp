// Initializes Rock Band services and loads config/rockband.dta. Returns true when initialization succeeds. Name is inferred from behavior.
bool game_initialize()
{
  __int64 v0; // rsi
  _BYTE v2[8]; // [rsp+8h] [rbp-28h] BYREF
  __int64 v3; // [rsp+10h] [rbp-20h]
  __int64 v4; // [rsp+18h] [rbp-18h]

  v4 = 0x6365786562696C2FLL;
  core_initialize();
  ui_register_layout_ids();
  stage_presence_register_ids();
  system_config_initialize("config/rockband.dta");
  sound_manager_initialize(0, 0xFFFFFFFF, 0, 0, 0);
  engine_register_types();
  animation_register_types();
  physics_register_types();
  game_audio_register_types();
  v2[0] = 1;
  v2[1] = 1;
  v2[2] = 1;
  v3 = 0;
  game_systems_initialize((__int64)v2, v0);
  ui_register_types();
  (*(void (**)(void))(g_dingo_service + 56))();
  audio_configure_time_stretch();
  input_refresh_player_assignments();
  if ( unk_19E4559 != 0 )
    runtime_terminate(0);
  ui_load_layout_by_id((__int64)&unk_1AFF588, 0x2Cu, 0);
  command_line_mark_switches_handled(arguments);
  return true;
}
