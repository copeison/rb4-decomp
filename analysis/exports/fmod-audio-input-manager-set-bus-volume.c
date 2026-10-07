void __fastcall fmod_audio_input_manager_set_bus_volume(__int64 a1, int a2, __m128 _XMM0)
{
  __int64 v3; // rax

  if ( a2 >= 0 )
  {
    v3 = *(_QWORD *)(a1 + 16);
    if ( (int)(-1431655765 * ((unsigned __int64)(*(_QWORD *)(a1 + 24) - v3) >> 3)) > a2 )
    {
      __asm { vmulss  xmm0, xmm0, dword ptr [rax+rcx*8+10h] }
      FMOD::Studio::Bus::setVolume(*(FMOD::Studio::Bus **)(v3 + 24LL * a2 + 8), *(float *)&_XMM0);
    }
  }
}
