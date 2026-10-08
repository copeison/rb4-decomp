// Renderer debug command handler inferred from the registration table.
__int64 __fastcall render_command_set_shading_mode(__int64 a1, __int64 a2)
{
  __int64 v3; // rdi

  v3 = *(_QWORD *)(g_render_system + 112);
  if ( v3 != 0 )
    render_command_set_shading_mode_for_owner(v3, a2);
  *(_QWORD *)a1 = 0;
  *(_DWORD *)(a1 + 8) = 0;
  return a1;
}
