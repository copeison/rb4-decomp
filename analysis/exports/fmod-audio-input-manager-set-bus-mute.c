void __fastcall fmod_audio_input_manager_set_bus_mute(__int64 a1, int a2, bool a3)
{
  __int64 v3; // rax

  if ( a2 >= 0 )
  {
    v3 = *(_QWORD *)(a1 + 16);
    if ( (int)(-1431655765 * ((unsigned __int64)(*(_QWORD *)(a1 + 24) - v3) >> 3)) > a2 )
      FMOD::Studio::Bus::setMute(*(FMOD::Studio::Bus **)(v3 + 24LL * a2 + 8), a3);
  }
}
