// Renderer debug command handler inferred from the registration table.
__int64 __fastcall render_command_take_screenshot(__int64 a1)
{
  screenshot_request();
  *(_QWORD *)a1 = 0;
  *(_DWORD *)(a1 + 8) = 0;
  return a1;
}
