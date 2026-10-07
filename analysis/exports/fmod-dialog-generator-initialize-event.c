char __fastcall fmod_dialog_generator_initialize_event(_QWORD *a1, __int64 a2, __int64 a3)
{
  _RBX = a1;
  if ( *(_DWORD *)(a3 + 96) != 4 )
    return 0;
  sub_1127330(a1, a2, a3);
  *((_DWORD *)_RBX + 102) = 1247525376;
  __asm
  {
    vmovups xmm0, xmmword ptr [rbx+40h]
    vmovups xmmword ptr [rbx+0C8h], xmm0
  }
  return fmod_studio_sound_generator_initialize_event(
           _RBX + 17,
           a2,
           a3,
           (__int64 (__fastcall *)())fmod_dialog_programmer_sound_callback,
           _RBX);
}
