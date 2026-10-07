int __fastcall fmod_system_callback(
        void *system,
        unsigned int type,
        void *command_data1,
        void *command_data2,
        void *user_data)
{
  __int64 v7; // rdi
  char *i; // rcx
  int v9; // eax
  int v10; // eax
  unsigned __int64 v11; // rax
  volatile signed __int32 *v13; // rcx
  bool v14; // cf
  unsigned __int32 v15; // eax
  unsigned __int32 v16; // ett
  char v18; // cc
  signed __int32 v19; // eax
  signed __int32 v20; // ett
  int v21; // eax
  int v22; // eax
  unsigned __int64 v23; // rax
  volatile signed __int32 *v25; // rcx
  bool v26; // cf
  unsigned __int32 v27; // eax
  unsigned __int32 v28; // ett
  signed __int32 v30; // eax
  signed __int32 v31; // ett
  char **v32; // rbx
  char *v33; // rdi
  char *v34; // rdi

  _R14 = (char *)user_data;
  if ( user_data == nullptr || *((_BYTE *)user_data + 308) != 0 || *((_QWORD *)user_data + 35) == 0 )
    return 0;
  if ( type == 64 )
  {
    if ( *((_BYTE *)user_data + 672) == 0 )
      return 0;
    v21 = *((_DWORD *)user_data + 130);
    if ( v21 > 0 )
    {
      v22 = v21 - 1;
      *((_DWORD *)user_data + 130) = v22;
      if ( v22 == 0 )
      {
        v23 = __rdtsc();
        *((_QWORD *)user_data + 64) += ((unsigned int)v23 | ((unsigned __int64)HIDWORD(v23) << 32))
                                     - *((_QWORD *)user_data + 63);
      }
    }
    *(double *)&_XMM0 = performance_counter_ticks_to_milliseconds(*((_QWORD *)user_data + 64));
    v25 = (volatile signed __int32 *)(_R14 + 496);
    v27 = _InterlockedCompareExchange((volatile signed __int32 *)_R14 + 124, 1, 0);
    v26 = v27 != 0;
    if ( v27 != 0 )
    {
      do
      {
        v28 = v27;
        v27 = _InterlockedCompareExchange(v25, 1, v27);
        v26 = v28 < v27;
      }
      while ( v28 != v27 );
    }
    __asm
    {
      vaddsd  xmm1, xmm0, qword ptr [r14+1D8h]
      vmovsd  qword ptr [r14+1D8h], xmm1
    }
    v18 = v26 | ((*((_DWORD *)_R14 + 120))++ == -1);
    __asm { vucomisd xmm0, qword ptr [r14+1E8h] }
    if ( !v18 )
      __asm { vmovsd  qword ptr [r14+1E8h], xmm0 }
    v30 = 1;
    do
    {
      v31 = v30;
      v30 = _InterlockedCompareExchange(v25, 0, v30);
    }
    while ( v31 != v30 );
    audio_rolling_timing_accumulator_finish(_R14 + 528);
    v32 = *((char ***)_R14 + 79);
    if ( v32 != (char **)(_R14 + 632) )
    {
      v33 = *((char **)_R14 + 79);
      do
      {
        (*(void (__fastcall **)(_QWORD *))(*((_QWORD *)v33 + 2) + 16LL))((_QWORD *)v33 + 2);
        v33 = *v32;
        v32 = (char **)v33;
      }
      while ( v33 != _R14 + 632 );
    }
    _R14[672] = 0;
    v34 = _R14 + 680;
LABEL_38:
    sem_post(v34);
    return 0;
  }
  if ( type != 32 )
    return 0;
  while ( (unsigned int)sem_wait(_R14 + 680) != 0 )
    _error(v7);
  if ( *((_QWORD *)_R14 + 35) == 0 )
  {
    v34 = _R14 + 680;
    goto LABEL_38;
  }
  _R14[672] = 1;
  ++*((_QWORD *)_R14 + 47);
  *((_QWORD *)_R14 + 54) = __rdtsc();
  *((_QWORD *)_R14 + 55) = 0;
  *((_DWORD *)_R14 + 112) = 1;
  *((_QWORD *)_R14 + 63) = __rdtsc();
  *((_QWORD *)_R14 + 64) = 0;
  *((_DWORD *)_R14 + 130) = 1;
  *((_QWORD *)_R14 + 72) = __rdtsc();
  *((_QWORD *)_R14 + 73) = 0;
  *((_DWORD *)_R14 + 148) = 1;
  for ( i = *((char **)_R14 + 79); i != _R14 + 632; i = *(char **)i )
  {
    *((_QWORD *)i + 9) = 0;
    *((_DWORD *)i + 20) = 0;
  }
  (*(void (__fastcall **)(char *, _QWORD))(*(_QWORD *)_R14 + 120LL))(_R14, *((_QWORD *)_R14 + 47));
  v9 = *((_DWORD *)_R14 + 112);
  if ( v9 > 0 )
  {
    v10 = v9 - 1;
    *((_DWORD *)_R14 + 112) = v10;
    if ( v10 == 0 )
    {
      v11 = __rdtsc();
      *((_QWORD *)_R14 + 55) += ((unsigned int)v11 | ((unsigned __int64)HIDWORD(v11) << 32)) - *((_QWORD *)_R14 + 54);
    }
  }
  *(double *)&_XMM0 = performance_counter_ticks_to_milliseconds(*((_QWORD *)_R14 + 55));
  v13 = (volatile signed __int32 *)(_R14 + 424);
  v15 = _InterlockedCompareExchange((volatile signed __int32 *)_R14 + 106, 1, 0);
  v14 = v15 != 0;
  if ( v15 != 0 )
  {
    do
    {
      v16 = v15;
      v15 = _InterlockedCompareExchange(v13, 1, v15);
      v14 = v16 < v15;
    }
    while ( v16 != v15 );
  }
  __asm
  {
    vaddsd  xmm1, xmm0, qword ptr [r14+190h]
    vmovsd  qword ptr [r14+190h], xmm1
  }
  v18 = v14 | ((*((_DWORD *)_R14 + 102))++ == -1);
  __asm { vucomisd xmm0, qword ptr [r14+1A0h] }
  if ( !v18 )
    __asm { vmovsd  qword ptr [r14+1A0h], xmm0 }
  v19 = 1;
  do
  {
    v20 = v19;
    v19 = _InterlockedCompareExchange(v13, 0, v19);
  }
  while ( v20 != v19 );
  return 0;
}
