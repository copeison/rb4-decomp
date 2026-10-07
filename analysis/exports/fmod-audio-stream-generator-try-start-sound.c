__int64 __fastcall fmod_audio_stream_generator_try_start_sound(__int64 a1)
{
  __int64 v2; // rdi
  int ChannelGroup; // eax
  int v4; // eax
  __int64 v5; // rdi
  __int64 v6; // rax
  int v7; // ecx
  char v10; // al
  int v12; // eax
  __int64 v13; // [rsp+0h] [rbp-30h] BYREF
  char v14; // [rsp+Eh] [rbp-22h] BYREF
  char v15; // [rsp+Fh] [rbp-21h] BYREF
  int v16; // [rsp+10h] [rbp-20h] BYREF
  int v17; // [rsp+14h] [rbp-1Ch] BYREF
  __int64 v18; // [rsp+18h] [rbp-18h]

  v18 = 0x6365786562696C2FLL;
  v16 = 0;
  v15 = 0;
  v14 = 0;
  FMOD::Sound::getOpenState(*(_QWORD *)(a1 + 80), &v17, &v16, &v15, &v14);
  v13 = 0;
  v2 = *(_QWORD *)(a1 + 96);
  if ( v2 != 0 )
  {
    ChannelGroup = FMOD::Studio::Bus::getChannelGroup(v2, &v13);
    if ( ChannelGroup != 0 )
    {
      if ( ChannelGroup == 76 )
      {
        v4 = *(_DWORD *)(a1 + 104);
        *(_DWORD *)(a1 + 104) = v4 - 1;
        if ( v4 > 1 )
          return 0x6365786562696C2FLL;
      }
      v13 = 0;
    }
  }
  if ( v17 == 0 )
  {
    FMOD::Sound::getLength(*(FMOD::Sound **)(a1 + 80), (unsigned int *)(a1 + 136), 1u);
    FMOD::Sound::getLength(*(FMOD::Sound **)(a1 + 80), (unsigned int *)(a1 + 272), 2u);
    v6 = *(_QWORD *)(a1 + 72);
    v7 = *(_DWORD *)(v6 + 16);
    if ( (unsigned int)(v7 - 1) <= 1 )
    {
      if ( v7 == 2 )
      {
        if ( *(_QWORD *)(v6 + 760) != 0 )
          v5 = *(_QWORD *)(v6 + 768);
      }
      else if ( *(_QWORD *)(v6 + 280) != 0 )
      {
        v5 = *(_QWORD *)(v6 + 288);
      }
    }
    FMOD::System::playSound(v5, *(_QWORD *)(a1 + 80), v13, 1, a1 + 88);
    FMOD::ChannelControl::setMode(*(FMOD::ChannelControl **)(a1 + 88), 2u);
    FMOD::Channel::setLoopCount(*(FMOD::Channel **)(a1 + 88), 0);
    FMOD::Channel::getFrequency(*(FMOD::Channel **)(a1 + 88), (float *)(a1 + 276));
    *(double *)&_XMM0 = (*(double (__fastcall **)(__int64))(*(_QWORD *)a1 + 104LL))(a1);
    __asm { vmulss  xmm0, xmm0, dword ptr [rbx+0CCh] }
    FMOD::ChannelControl::setVolume(*(FMOD::ChannelControl **)(a1 + 88), *(float *)&_XMM0);
    if ( *(_BYTE *)(a1 + 256) != 0 )
    {
      *(_DWORD *)(a1 + 28) = 4;
    }
    else
    {
      FMOD::ChannelControl::setPaused(*(FMOD::ChannelControl **)(a1 + 88), false);
      v10 = *(_BYTE *)(a1 + 256);
      *(_DWORD *)(a1 + 28) = (v10 != 0) + 3;
      if ( v10 == 0 )
      {
        v12 = *(_DWORD *)(a1 + 128);
        if ( v12 >= 0 )
        {
          *(_DWORD *)(a1 + 128) = v12 + 1;
          if ( v12 == 0 )
            *(_QWORD *)(a1 + 112) = __rdtsc();
        }
      }
    }
  }
  return 0x6365786562696C2FLL;
}
