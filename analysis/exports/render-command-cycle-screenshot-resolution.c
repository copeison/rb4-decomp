// Renderer debug command handler inferred from the registration table.
__int64 __fastcall render_command_cycle_screenshot_resolution(__int64 a1)
{
  __int64 v2; // rax
  __int64 v3; // rdi

  v2 = *(_QWORD *)(g_render_system + 296);
  v3 = (unsigned int)(*(_DWORD *)(v2 + 184) - 6 * ((*(_DWORD *)(v2 + 184) + 1) / 6) + 1);
  *(_DWORD *)(v2 + 184) = v3;
  sub_43AF40(v3);
  *(_QWORD *)a1 = 0;
  *(_DWORD *)(a1 + 8) = 0;
  return a1;
}
