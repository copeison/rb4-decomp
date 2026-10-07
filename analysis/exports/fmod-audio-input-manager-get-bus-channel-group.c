__int64 __fastcall fmod_audio_input_manager_get_bus_channel_group(__int64 a1, int a2)
{
  __int64 v2; // rax
  int ChannelGroup; // ecx
  __int64 result; // rax
  _QWORD v5[3]; // [rsp+8h] [rbp-18h] BYREF

  v5[1] = 0x6365786562696C2FLL;
  if ( a2 < 0 )
    return 0;
  v2 = *(_QWORD *)(a1 + 16);
  if ( (int)(-1431655765 * ((unsigned __int64)(*(_QWORD *)(a1 + 24) - v2) >> 3)) <= a2 )
    return 0;
  ChannelGroup = FMOD::Studio::Bus::getChannelGroup(*(_QWORD *)(v2 + 24LL * a2 + 8), v5);
  result = 0;
  if ( ChannelGroup == 0 )
    return v5[0];
  return result;
}
