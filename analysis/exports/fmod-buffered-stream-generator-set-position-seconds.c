__int64 __fastcall fmod_buffered_stream_generator_set_position_seconds(__int64 a1, __m128 _XMM0)
{
  _QWORD *v3; // r14
  __int64 v5; // rax
  __int64 v10; // rbx
  __int64 v11; // r13
  char v12; // al
  int v13; // ecx
  int v14; // edi
  unsigned __int64 v15; // r13
  int v16; // eax
  int v17; // ecx
  int v18; // esi
  int v19; // r12d
  unsigned int v21; // edx
  __int64 v22; // r13
  int v23; // r12d
  __int64 i; // r14
  _QWORD *v27; // [rsp+0h] [rbp-30h]

  _R15 = a1;
  __asm { vmovss  dword ptr [rbp+var_30], xmm0 }
  v3 = (_QWORD *)(a1 + 80);
  (*(void (__fastcall **)(__int64))(*(_QWORD *)(a1 + 80) + 40LL))(a1 + 80);
  if ( *(_BYTE *)(_R15 + 480) != 0 )
  {
    v5 = *v3;
  }
  else
  {
    __asm
    {
      vmovss  xmm0, dword ptr [rbp+var_30]
      vmovss  dword ptr [r15+1E4h], xmm0
      vmulss  xmm0, xmm0, cs:dword_125CD58
      vmulss  xmm0, xmm0, dword ptr [r15+1D4h]
      vcvttss2si r12d, xmm0
    }
    while ( 1 )
    {
      v10 = *(_QWORD *)(_R15 + 280);
      v11 = *(_QWORD *)(_R15 + 288);
      if ( v10 == v11 )
        break;
      v12 = 1;
      do
      {
        v13 = *(_DWORD *)(v10 + 64);
        if ( v13 != 0 && v13 != 3 )
        {
          sub_264700(*(_QWORD *)(_R15 + 16) + 72LL, v10);
          v12 = 0;
        }
        v10 += 192;
      }
      while ( v11 != v10 );
      if ( (v12 & 1) != 0 )
      {
        v10 = *(_QWORD *)(_R15 + 280);
        v11 = *(_QWORD *)(_R15 + 288);
        break;
      }
      usleep(0);
    }
    *(_BYTE *)(_R15 + 492) = 1;
    v14 = _R12D;
    v15 = (unsigned __int64)(v11 - v10) >> 6;
    v16 = -1431655765 * (int)v15 / 2;
    *(_DWORD *)(_R15 + 488) = v16;
    *(_DWORD *)(_R15 + 500) = 0;
    v17 = *(_DWORD *)(_R15 + 448);
    v18 = *(_DWORD *)(_R15 + 452);
    v19 = _R12D - v18 * v17;
    if ( v19 < 0 )
    {
      v19 = 0;
      v16 = v14 / v17;
      *(_DWORD *)(_R15 + 488) = v14 / v17;
      __asm
      {
        vcvtsi2ss xmm0, xmm1, edi
        vmovss  dword ptr [r15+1F4h], xmm0
      }
    }
    v21 = (v16 - v18) & ~((v16 - v18) >> 31);
    if ( (int)v21 < -1431655765 * (int)v15 )
    {
      v22 = v21;
      v27 = v3;
      v23 = v19 - v17;
      for ( i = 192LL * v21; ; i += 192 )
      {
        ++v22;
        v23 += v17;
        *(_DWORD *)(v10 + i + 136) = v23;
        *(_DWORD *)(v10 + i) = v23;
        *(_DWORD *)(v10 + i + 4) = *(_DWORD *)(_R15 + 448);
        *(_DWORD *)(v10 + i + 12) = 0;
        sub_264660(*(_QWORD *)(_R15 + 16) + 72LL, v10 + i);
        v10 = *(_QWORD *)(_R15 + 280);
        if ( v22 >= (int)(-1431655765 * ((unsigned __int64)(*(_QWORD *)(_R15 + 288) - v10) >> 6)) )
          break;
        v17 = *(_DWORD *)(_R15 + 448);
      }
      v18 = *(_DWORD *)(_R15 + 452);
      v16 = *(_DWORD *)(_R15 + 488);
      v3 = v27;
    }
    *(_DWORD *)(_R15 + 496) = v16 + v18;
    v5 = *(_QWORD *)(_R15 + 80);
  }
  return (*(__int64 (__fastcall **)(_QWORD *))(v5 + 56))(v3);
}
