// Locks the render system, records the calling thread, updates frame timing, and begins GPU frame tracking.
__int64 __fastcall render_system_prepare_frame(__int64 a1, unsigned __int8 a2)
{
  __int64 v4; // rdi
  unsigned __int64 v6; // rdx
  unsigned __int64 v7; // rdi
  char v15; // zf
  int v18; // eax
  __int64 v19; // rbx
  _QWORD v21[6]; // [rsp+0h] [rbp-30h] BYREF

  _R15 = a1;
  v21[1] = 0x6365786562696C2FLL;
  scePthreadMutexLock(a1 + 16);
  ++*(_DWORD *)(_R15 + 8);
  *(_QWORD *)(_R15 + 24) = scePthreadSelf(v4);
  *(_BYTE *)(_R15 + 176) = 1;
  (*(void (__fastcall **)(__int64, _QWORD))(*(_QWORD *)_R15 + 48LL))(_R15, a2);
  if ( a2 == 0 )
  {
    if ( (*(unsigned int (__fastcall **)(__int64))(*(_QWORD *)_R15 + 104LL))(_R15) == 0 )
    {
      sub_249D40(&unk_19E7CD0, 0);
      if ( *(_BYTE *)(*(_QWORD *)(_R15 + 296) + 180LL) != 0 )
        unk_19E7E60 = *(_QWORD *)(_R15 + 160) & 1LL;
    }
    v6 = __rdtsc();
    if ( *(_DWORD *)(_R15 + 272) != 0 )
      v7 = v6 - *(_QWORD *)(_R15 + 256);
    else
      v7 = *(_QWORD *)(_R15 + 264);
    *(_QWORD *)(_R15 + 256) = v6;
    *(_QWORD *)(_R15 + 264) = 0;
    *(_DWORD *)(_R15 + 272) = 1;
    *(double *)&_XMM0 = performance_counter_ticks_to_milliseconds(v7);
    __asm
    {
      vmovss  xmm1, cs:dword_127E1C0
      vcvtsd2ss xmm0, xmm0, xmm0
      vxorps  xmm2, xmm2, xmm2
      vdivss  xmm0, xmm1, xmm0
      vmovss  dword ptr [r15+120h], xmm0
      vmovss  xmm1, dword ptr [r15+124h]
      vucomiss xmm2, xmm1
    }
    if ( v15 )
    {
      __asm
      {
        vmovaps xmm1, xmm0
        vmovss  dword ptr [r15+124h], xmm0
      }
    }
    __asm
    {
      vmulss  xmm1, xmm1, cs:dword_127E1C4
      vaddss  xmm0, xmm1, xmm0
      vmulss  xmm0, xmm0, cs:dword_127E1C8
      vmovss  dword ptr [r15+124h], xmm0
    }
  }
  *(_BYTE *)(*(_QWORD *)(_R15 + 56) + 8LL) = 1;
  if ( byte_1A712B0 == 0 )
  {
    *(double *)_XMM0.m128_u64 = _cxa_guard_acquire(&byte_1A712B0);
    if ( v18 != 0 )
    {
      qword_1A712A8 = 19246190;
      *(double *)_XMM0.m128_u64 = _cxa_guard_release(&byte_1A712B0);
    }
  }
  if ( qword_1A712A8 == 19246190 )
  {
    *(double *)_XMM0.m128_u64 = sub_256FD0(v21, "GPU Total");
    qword_1A712A8 = v21[0];
  }
  v19 = g_render_system;
  if ( *(_BYTE *)(g_render_system + 64) != 0 )
  {
    sub_6BC3B0(*(_QWORD *)(g_render_system + 56), *(_DWORD *)(g_render_system + 68), _XMM0);
    *(_BYTE *)(v19 + 64) = 0;
    *(_DWORD *)(v19 + 68) = 0;
  }
  *(_QWORD *)(_R15 + 280) = sub_62AF80(_R15 + 3584, *(_QWORD *)(v19 + 56));
  sub_457780(*(_QWORD *)(_R15 + 3576), *(_QWORD *)(_R15 + 56));
  sub_5F2B40(unk_1AA76F8, *(_QWORD *)(_R15 + 56));
  return 0x6365786562696C2FLL;
}
