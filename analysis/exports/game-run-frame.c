// Runs one main-loop iteration across input, platform, UI, audio, and game systems. Returns whether another frame should run. Name is inferred from its sole looping caller.
bool game_run_frame()
{
  __int64 v0; // rax
  __int64 v1; // rdi
  __int64 v2; // rsi
  _QWORD v4[4]; // [rsp+0h] [rbp-60h] BYREF
  _QWORD *v5; // [rsp+20h] [rbp-40h]
  __int64 v6; // [rsp+38h] [rbp-28h]

  v6 = 0x6365786562696C2FLL;
  system_update();
  sound_manager_update(&g_sound_manager);
  if ( unk_1ADF350 != 0 )
    sub_8ECAC0();
  dingo_update(&g_dingo_service);
  async_callback_queue_update(&unk_1B15288);
  somp_session_manager_update(&dword_1B156A0);
  sub_F2CE50(unk_1B43A10);
  sub_AE3880(unk_1AF4E38);
  sub_D1E2C0(&unk_1B15110);
  song_loading_update(unk_1ADF7D0);
  profile_manager_update(unk_1B1F7F0);
  sub_ADE120();
  resource_manager_update(unk_1ADFE28);
  sub_B0A070();
  ui_manager_update(g_ui_manager);
  if ( unk_1AEF778 != 0 )
    sub_A47EC0();
  ui_layout_controller_update(&g_ui_layout_controller);
  sub_3AE960(unk_1A6E0A8);
  sub_C60720(unk_1B094D8);
  sub_B0CE80(unk_1AFA328);
  network_connection_monitor_update(unk_1B01798);
  sub_928FC0(&unk_1ADFB78);
  render_system_poll(g_render_system);
  if ( g_render_system == 0 )
    return false;
  v0 = ui_manager_get_active_layout(g_ui_manager);
  if ( v0 != 0 && (unsigned __int8)ui_layout_consume_skip_frame(v0) != 0 )
  {
    render_system_skip_frame(g_render_system);
  }
  else
  {
    if ( (unsigned __int8)sub_43B130() != 0 )
    {
      v1 = *(_QWORD *)(g_render_system + 112LL);
      v2 = *(unsigned int *)(*(_QWORD *)(g_render_system + 296LL) + 184LL);
      v4[0] = &unk_18DC010;
      v5 = v4;
      sub_43B140(v1, v2, v4);
      if ( v5 != nullptr )
      {
        (*(void (__fastcall **)(_QWORD *, bool))(*v5 + 32LL))(v5, v5 != v4);
        v5 = nullptr;
      }
    }
    if ( (unsigned __int8)render_system_begin_frame(g_render_system) != 0 )
    {
      ui_manager_render(g_ui_manager);
      render_system_end_frame(g_render_system);
    }
  }
  return g_exit_requested == 0 && g_render_system != 0;
}
