__int64 __fastcall fmod_sound_to_pcm_callback_run(__int64 a1)
{
  unsigned __int64 v2; // r15
  unsigned __int64 v3; // rbx
  unsigned __int64 v4; // r14
  char v5; // al
  double Length; // xmm0_8
  int v8; // eax
  unsigned int v9; // r15d
  __int64 v10; // rdi
  unsigned int v11; // ebx
  unsigned int v12; // r12d
  unsigned __int8 v13; // al
  __int64 v14; // rax
  int v16; // [rsp+4h] [rbp-4Ch] BYREF
  int v17; // [rsp+8h] [rbp-48h] BYREF
  _BYTE v18[4]; // [rsp+Ch] [rbp-44h] BYREF
  unsigned int v19; // [rsp+10h] [rbp-40h] BYREF
  int v20; // [rsp+14h] [rbp-3Ch] BYREF
  unsigned int v21; // [rsp+18h] [rbp-38h] BYREF
  unsigned int v22; // [rsp+1Ch] [rbp-34h] BYREF
  __int64 v23; // [rsp+20h] [rbp-30h]

  v2 = 0;
  v23 = 0x6365786562696C2FLL;
  v22 = 0;
  v20 = 0;
  LOBYTE(v19) = 0;
  v18[0] = 0;
  v3 = __rdtsc();
  while ( 1 )
  {
    v4 = v3;
    FMOD::Sound::getOpenState(*(_QWORD *)(a1 + 24), &v21, &v20, &v19, v18);
    v3 = __rdtsc();
    v2 += v3 - v4;
    performance_counter_ticks_to_milliseconds(v2);
    v5 = *(_BYTE *)(a1 + 49);
    if ( v21 == 0 )
      break;
    if ( v5 != 0 )
    {
      v5 = 1;
      break;
    }
  }
  if ( v5 == 0 )
  {
    FMOD::Sound::getFormat(*(_QWORD *)(a1 + 24), &v21, &v20, &v19, v18);
    FMOD::Sound::getDefaults(*(FMOD::Sound **)(a1 + 24), (float *)&v17, &v16);
    Length = FMOD::Sound::getLength(*(FMOD::Sound **)(a1 + 24), &v22, 2u);
    __asm { vcvttss2si esi, [rbp+var_48] }
    v8 = (*(__int64 (__fastcall **)(_QWORD, __int64, __int64, _QWORD, _QWORD, double))(**(_QWORD **)(a1 + 16) + 64LL))(
           *(_QWORD *)(a1 + 16),
           _RSI,
           2,
           v19,
           v22,
           Length);
    *(_DWORD *)(a1 + 40) = v8;
    v9 = 2 * v19;
    v10 = 2 * v19 * v8;
    *(_DWORD *)(a1 + 44) = v10;
    *(_QWORD *)(a1 + 32) = sub_37AE70(v10, "FMODSoundToPCMCallback", 8);
    v11 = 0;
    FMOD::Sound::seekData(*(FMOD::Sound **)(a1 + 24), 0);
    v21 = 0;
    do
    {
      FMOD::Sound::readData(*(FMOD::Sound **)(a1 + 24), *(void **)(a1 + 32), *(_DWORD *)(a1 + 44), &v21);
      v12 = v21 / v9;
      v13 = (*(__int64 (__fastcall **)(_QWORD, _QWORD, _QWORD))(**(_QWORD **)(a1 + 16) + 72LL))(
              *(_QWORD *)(a1 + 16),
              *(_QWORD *)(a1 + 32),
              v21);
      if ( v21 == 0 )
        break;
      if ( (v13 & (*(_BYTE *)(a1 + 49) == 0)) != 1 )
        break;
      v11 += v12;
    }
    while ( v11 < v22 );
    if ( v13 == 0 )
    {
      *(_BYTE *)(*(_QWORD *)(a1 + 8) + 64LL) = 1;
      *(_BYTE *)(a1 + 49) = 1;
    }
    FMOD::Sound::seekData(*(FMOD::Sound **)(a1 + 24), 0);
    v14 = **(_QWORD **)(a1 + 16);
    if ( *(_BYTE *)(a1 + 49) != 0 )
      (*(void (**)(void))(v14 + 88))();
    else
      (*(void (**)(void))(v14 + 80))();
    *(_QWORD *)(a1 + 16) = 0;
  }
  return 0;
}
