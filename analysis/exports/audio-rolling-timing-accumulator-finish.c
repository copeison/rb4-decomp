void __fastcall audio_rolling_timing_accumulator_finish(void *timing)
{
  int v2; // eax
  int v3; // eax
  unsigned __int64 v4; // rax
  volatile signed __int32 *v6; // rsi
  signed __int32 v7; // eax
  signed __int32 v8; // ett
  int v10; // ecx
  bool v16; // zf
  signed __int32 v17; // eax
  signed __int32 v18; // ett

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
  *(double *)&_XMM0 = performance_counter_ticks_to_milliseconds(*((_QWORD *)timing + 7));
  v6 = _RBX + 10;
  v7 = _InterlockedCompareExchange(_RBX + 10, 1, 0);
  if ( v7 != 0 )
  {
    do
    {
      v8 = v7;
      v7 = _InterlockedCompareExchange(v6, 1, v7);
    }
    while ( v8 != v7 );
  }
  _RDI = *((_QWORD *)_RBX + 11);
  v10 = *((_DWORD *)_RBX + 20) + 1;
  _RDX = (unsigned int)(*((_DWORD *)_RBX + 20) % *((_DWORD *)_RBX + 24));
  __asm
  {
    vsubsd  xmm1, xmm0, qword ptr [rdi+rdx*8]
    vaddsd  xmm1, xmm1, qword ptr [rbx+48h]
    vmovsd  qword ptr [rbx+48h], xmm1
    vmovsd  qword ptr [rdi+rdx*8], xmm0
  }
  __asm
  {
    vmovsd  xmm0, qword ptr [rbx+48h]
    vaddsd  xmm1, xmm0, qword ptr [rbx+10h]
    vmovsd  qword ptr [rbx+10h], xmm1
  }
  v16 = (*((_DWORD *)_RBX + 6))++ == -1;
  *((_DWORD *)_RBX + 20) = v10;
  __asm { vucomisd xmm0, qword ptr [rbx+20h] }
  if ( !v16 )
    __asm { vmovsd  qword ptr [rbx+20h], xmm0 }
  v17 = _InterlockedCompareExchange(v6, 0, 1);
  if ( v17 != 1 )
  {
    do
    {
      v18 = v17;
      v17 = _InterlockedCompareExchange(v6, 0, v17);
    }
    while ( v18 != v17 );
  }
}
