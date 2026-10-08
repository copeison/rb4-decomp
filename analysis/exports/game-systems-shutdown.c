// Reentrancy-guarded cleanup callback for render-dependent game systems and the global render-system object.
char __fastcall game_systems_shutdown(__int64 a1)
{
  char result; // al
  __int64 v2; // rdi
  __int64 v3; // rdi
  __int64 v4; // rdi
  __int64 v5; // rdi
  __int64 v6; // rdi
  __int64 v7; // rdi
  __int64 v8; // rdi

  if ( g_render_system != 0 )
  {
    result = byte_1A71914;
    if ( byte_1A71914 == 0 )
    {
      byte_1A71914 = 1;
      sub_6B54E0(a1);
      sub_65C790(v2);
      sub_3DF820(v3);
      sub_461F50(v4);
      sub_5F7FB0(v5);
      sub_601910(v6);
      sub_44C930(v7);
      sub_65CE70(v8);
      result = render_system_shutdown(g_render_system);
      if ( g_render_system != 0 )
        result = (*(__int64 (__fastcall **)(__int64))(*(_QWORD *)g_render_system + 8LL))(g_render_system);
      g_render_system = 0;
      byte_1A71914 = 0;
    }
  }
  return result;
}
