__int64 __fastcall fmod_buffered_stream_generator_return_to_pool(__int64 a1, __m128 _XMM0)
{
  __int64 v4; // rdi
  __int64 v5; // rbx
  __int64 v6; // r12
  char v7; // al
  int v8; // ecx
  _QWORD *v9; // r12
  __int64 v10; // rax
  _QWORD *v11; // rax
  _QWORD **v12; // rcx
  __int64 v13; // rdx
  _QWORD *v14; // rdx
  _QWORD *v15; // rdi
  __int64 v20; // rax
  __int64 v22; // rbx
  __int64 v23; // rax
  _QWORD *v24; // rcx
  _QWORD v26[7]; // [rsp+8h] [rbp-38h] BYREF

  _R14 = a1;
  v26[2] = 0x6365786562696C2FLL;
  v4 = *(_QWORD *)(a1 + 312);
  if ( v4 != 0 )
  {
    (*(void (__fastcall **)(__int64, double))(*(_QWORD *)v4 + 184LL))(v4, *(double *)_XMM0.m128_u64);
    *(_QWORD *)(_R14 + 312) = 0;
  }
  while ( 1 )
  {
    v5 = *(_QWORD *)(_R14 + 280);
    v6 = *(_QWORD *)(_R14 + 288);
    if ( v5 == v6 )
      break;
    v7 = 1;
    do
    {
      v8 = *(_DWORD *)(v5 + 64);
      if ( v8 != 0 && v8 != 3 )
      {
        sub_264700(*(_QWORD *)(_R14 + 16) + 72LL, v5);
        v7 = 0;
      }
      v5 += 192;
    }
    while ( v6 != v5 );
    if ( (v7 & 1) != 0 )
    {
      v9 = *(_QWORD **)(_R14 + 280);
      v5 = *(_QWORD *)(_R14 + 288);
      if ( v9 != (_QWORD *)v5 )
      {
        do
        {
          v10 = v9[23];
          if ( v10 != 0 )
          {
            v9[23] = 0;
            --*(_QWORD *)(v10 + 16);
            v11 = v9 + 21;
            v12 = (_QWORD **)(v9 + 22);
            v13 = v9[21];
            *(_QWORD *)(v13 + 8) = v9[22];
            *(_QWORD *)v9[22] = v13;
            v14 = v9 + 21;
            v9[21] = v9 + 21;
            v9[22] = v9 + 21;
          }
          else
          {
            v11 = (_QWORD *)v9[21];
            v14 = (_QWORD *)v9[22];
            v12 = (_QWORD **)(v9 + 22);
          }
          v11[1] = v14;
          **v12 = v11;
          v15 = (_QWORD *)v9[14];
          if ( v15 != nullptr )
          {
            (*(void (__fastcall **)(_QWORD *, bool, double))(*v15 + 32LL))(
              v15,
              v9 + 10 != v15,
              *(double *)_XMM0.m128_u64);
            v9[14] = 0;
          }
          v9 += 24;
        }
        while ( (_QWORD *)v5 != v9 );
        v5 = *(_QWORD *)(_R14 + 280);
      }
      break;
    }
    usleep(0);
  }
  *(_QWORD *)(_R14 + 288) = v5;
  _RBX = (_QWORD *)(_R14 + 344);
  if ( *(_DWORD *)(_R14 + 336) == 0 )
  {
    sub_37B800(*_RBX - 2LL, *(double *)_XMM0.m128_u64);
    *_RBX = 0;
  }
  __asm { vxorps  ymm0, ymm0, ymm0 }
  __asm
  {
    vmovups ymmword ptr [rbx+20h], ymm0
    vmovups ymmword ptr [rbx], ymm0
    vxorps  xmm0, xmm0, xmm0
  }
  *(double *)&_XMM0 = sub_D3D20(v26, 0, 0, 0, *(double *)&_XMM0);
  v20 = v26[0];
  __asm { vxorps  xmm0, xmm0, xmm0 }
  *(_QWORD *)(_R14 + 413) = *(_QWORD *)((char *)v26 + 5);
  *(_QWORD *)(_R14 + 408) = v20;
  *(_DWORD *)(_R14 + 336) = 1;
  __asm { vmovups xmmword ptr [r14+1A8h], xmm0 }
  *(_DWORD *)(_R14 + 440) = 0;
  *(_BYTE *)(_R14 + 444) = 1;
  FMOD::Sound::release(*(FMOD::Sound **)(_R14 + 456));
  v22 = *(_QWORD *)(_R14 + 16);
  scePthreadMutexLock(v22 + 24);
  ++*(_DWORD *)(v22 + 16);
  *(_BYTE *)(_R14 + 39) &= ~0x80u;
  *(_QWORD *)(_R14 + 64) = 0;
  v23 = v22 + 32;
  if ( *(_QWORD *)(_R14 + 56) != v22 + 32 )
  {
    *(_QWORD *)(_R14 + 56) = v23;
    ++*(_QWORD *)(v22 + 48);
    v24 = *(_QWORD **)(v22 + 40);
    *(_QWORD *)(_R14 + 48) = v24;
    *(_QWORD *)(_R14 + 40) = v23;
    *v24 = _R14 + 40;
    *(_QWORD *)(v22 + 40) = _R14 + 40;
  }
  --*(_DWORD *)(v22 + 16);
  scePthreadMutexUnlock(v22 + 24);
  return 0x6365786562696C2FLL;
}
