void __fastcall audio_timing_accumulator_finish(void *timing)
{
  int v2; // eax
  int v3; // eax
  unsigned __int64 v4; // rax
  volatile signed __int32 *v6; // rcx
  bool v7; // cf
  unsigned __int32 v8; // eax
  unsigned __int32 v9; // ett
  char v11; // cc
  signed __int32 v12; // eax
  signed __int32 v13; // ett

  _RBX = (volatile signed __int32 *)timing;
  v2 = *((_DWORD *)timing + 16);
  if ( v2 > 0 )
  {
    v3 = v2 - 1;
    *((_DWORD *)timing + 16) = v3;
    if ( v3 == 0 )
    {
      v4 = __rdtsc();
      *((_QWORD *)timing + 7) += ((unsigned int)v4 | ((unsigned __int64)HIDWORD(v4) << 32)) - *((_QWORD *)timing + 6);
    }
  }
  *(double *)&_XMM0 = performance_counter_ticks_to_seconds(*((_QWORD *)timing + 7));
  v6 = _RBX + 10;
  v8 = _InterlockedCompareExchange(_RBX + 10, 1, 0);
  v7 = v8 != 0;
  if ( v8 != 0 )
  {
    do
    {
      v9 = v8;
      v8 = _InterlockedCompareExchange(v6, 1, v8);
      v7 = v9 < v8;
    }
    while ( v9 != v8 );
  }
  __asm
  {
    vaddsd  xmm1, xmm0, qword ptr [rbx+10h]
    vmovsd  qword ptr [rbx+10h], xmm1
  }
  v11 = v7 | ((*((_DWORD *)_RBX + 6))++ == -1);
  __asm { vucomisd xmm0, qword ptr [rbx+20h] }
  if ( !v11 )
    __asm { vmovsd  qword ptr [rbx+20h], xmm0 }
  v12 = 1;
  do
  {
    v13 = v12;
    v12 = _InterlockedCompareExchange(v6, 0, v12);
  }
  while ( v13 != v12 );
}
