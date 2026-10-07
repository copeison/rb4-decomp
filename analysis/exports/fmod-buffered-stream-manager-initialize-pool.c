__int64 __fastcall fmod_buffered_stream_manager_initialize_pool(__int64 a1, __int64 a2)
{
  unsigned __int64 v4; // rbx
  unsigned __int128 v5; // rax
  __int64 v6; // rdi
  unsigned __int64 *v7; // rax
  __int64 v8; // rsi
  _QWORD *v10; // r15
  __int64 v13; // rcx
  __int64 v18; // rax
  bool v20; // cc
  __int64 v21; // r14
  __int64 v22; // r13
  __int64 v23; // rbx
  _QWORD *v24; // rcx
  _QWORD v26[8]; // [rsp+20h] [rbp-40h] BYREF

  v26[2] = 0x6365786562696C2FLL;
  v4 = *(int *)(a1 + 8);
  v5 = 0x238 * (unsigned __int128)v4;
  *(_QWORD *)&v5 = 568 * v4 + 8;
  v6 = -1;
  if ( 568 * v4 >= 0xFFFFFFFFFFFFFFF8LL )
    *(_QWORD *)&v5 = -1;
  if ( !__OFADD__(127, (0x238 * (unsigned __int128)v4) >> 64 != 0) )
    v6 = v5;
  v7 = (unsigned __int64 *)sub_37BF60(v6, a2, *((_QWORD *)&v5 + 1));
  v10 = v7 + 1;
  *v7 = v4;
  if ( v4 != 0 )
  {
    _R13 = v7 + 1;
    do
    {
      __asm { vxorps  xmm0, xmm0, xmm0 }
      *_R13 = &unk_18DCD58;
      _R13[1] = 19246190;
      _R13[2] = 0;
      _R13[3] = 0x500000000LL;
      v13 = (unsigned int)_InterlockedExchange((volatile __int32 *)_R13 + 8, 0);
      *((_DWORD *)_R13 + 9) = -1;
      _R13[6] = _R13 + 5;
      _R13[5] = _R13 + 5;
      __asm { vmovups xmmword ptr [r13+38h], xmm0 }
      sub_BEF90(_R13 + 10, v8, 0x500000000LL, v13);
      __asm { vxorps  ymm0, ymm0, ymm0 }
      *_R13 = &unk_18F0668;
      _R13[10] = &unk_18F0780;
      __asm { vmovups ymmword ptr [r13+110h], ymm0 }
      nullsub_18(_R13 + 38, "EASTL vector");
      _R13[41] = _R13 + 40;
      _R13[40] = _R13 + 40;
      *(double *)&_XMM0 = sub_D3D10(_R13 + 51);
      __asm { vxorps  ymm0, ymm0, ymm0 }
      __asm
      {
        vmovups ymmword ptr [r13+178h], ymm0
        vmovups ymmword ptr [r13+158h], ymm0
        vxorps  xmm0, xmm0, xmm0
      }
      *(double *)&_XMM0 = sub_D3D20(v26, 0, 0, 0, *(double *)&_XMM0);
      v18 = v26[0];
      __asm { vxorps  xmm0, xmm0, xmm0 }
      *(_QWORD *)((char *)_R13 + 413) = *(_QWORD *)((char *)v26 + 5);
      _R13[51] = v18;
      *((_DWORD *)_R13 + 84) = 1;
      __asm { vmovups xmmword ptr [r13+1A8h], xmm0 }
      *((_DWORD *)_R13 + 110) = 0;
      *((_BYTE *)_R13 + 444) = 1;
      sub_1172A10((char *)_R13 + 540);
      _R13 += 71;
    }
    while ( _R13 != &v10[71 * v4] );
    v20 = *(_DWORD *)(a1 + 8) <= 0;
    *(_QWORD *)(a1 + 64) = v10;
    if ( !v20 )
    {
      v21 = a1 + 32;
      v22 = 0;
      v23 = 5;
      do
      {
        (*(void (__fastcall **)(_QWORD *, __int64, _QWORD))(v10[v23 - 5] + 152LL))(&v10[v23 - 5], a1, (unsigned int)v22);
        v10 = *(_QWORD **)(a1 + 64);
        ++v22;
        v10[v23 + 2] = v21;
        ++*(_QWORD *)(a1 + 48);
        v24 = *(_QWORD **)(a1 + 40);
        v10[v23 + 1] = v24;
        v10[v23] = v21;
        *v24 = &v10[v23];
        *(_QWORD *)(a1 + 40) = &v10[v23];
        v23 += 71;
      }
      while ( v22 < *(int *)(a1 + 8) );
    }
  }
  else
  {
    *(_QWORD *)(a1 + 64) = v10;
  }
  return 0x6365786562696C2FLL;
}
