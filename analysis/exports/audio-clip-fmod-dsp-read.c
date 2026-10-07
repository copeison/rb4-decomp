__int64 __fastcall audio_clip_fmod_dsp_read(
        FMOD::DSP **a1,
        unsigned int a2,
        __int64 a3,
        __int64 a4,
        __int64 a5,
        int a6)
{
  FMOD::DSP *v9; // rdi
  __int64 v10; // r13
  int v12; // eax
  _BYTE *v13; // r11
  unsigned __int64 v14; // r8
  unsigned __int64 v15; // r9
  __int64 v16; // rdx
  __int64 v18; // r15
  __int64 v19; // r10
  unsigned __int64 v20; // rsi
  char v21; // bl
  unsigned __int8 v22; // si
  __int64 v24; // rbx
  _DWORD *v25; // rsi
  int *v36; // rcx
  int *v37; // rdi
  __int64 v38; // rdx
  int v39; // ebx
  int v40; // ebx
  unsigned __int64 v42; // [rsp+0h] [rbp-40h]
  void *v43[7]; // [rsp+8h] [rbp-38h] BYREF

  v43[1] = (void *)0x6365786562696C2FLL;
  if ( a6 != 1 )
  {
    v9 = *a1;
    v43[0] = nullptr;
    if ( (unsigned int)FMOD::DSP::getUserData(v9, v43) != 0 || v43[0] == nullptr )
    {
      memset(**(_QWORD **)(a4 + 24), 0, 8LL * a2);
      return 0;
    }
    v10 = *((_QWORD *)v43[0] + 15);
    scePthreadMutexLock(v10 + 40);
    v12 = *(_DWORD *)(v10 + 32) + 1;
    *(_DWORD *)(v10 + 32) = v12;
    v13 = v43[0];
    if ( *((_BYTE *)v43[0] + 253) == 0 )
    {
      memset(**(_QWORD **)(a4 + 24), 0, 8LL * a2);
      v12 = *(_DWORD *)(v10 + 32);
LABEL_23:
      *(_DWORD *)(v10 + 32) = v12 - 1;
      scePthreadMutexUnlock(v10 + 40);
      return 0;
    }
    if ( a2 == 0 )
    {
LABEL_22:
      v13[253] = 0;
      goto LABEL_23;
    }
    v14 = *((_QWORD *)v43[0] + 19);
    v15 = *((_QWORD *)v43[0] + 20);
    v16 = a2;
    _RDI = **(_QWORD **)(a4 + 24);
    if ( a2 > 7 )
    {
      v18 = a2 & 7;
      v19 = v16 - v18;
      if ( v16 != v18 )
      {
        v20 = _RDI + 8 * v16;
        v42 = *((_QWORD *)v43[0] + 19);
        v21 = _RDI < v14 + 4 * v16 ? -(v42 < v20) : 0;
        v22 = -(v15 < v20);
        if ( (v21 & 1) == 0 && (v22 & (unsigned __int8)-(_RDI < v15 + 4 * v16) & 1) == 0 )
        {
          _R8 = *((_QWORD *)v43[0] + 19);
          v24 = v16 - v18;
          v25 = (_DWORD *)(_RDI + 4 * (2 * v16 - (unsigned int)(2 * v18)));
          _RCX = *((_QWORD *)v43[0] + 20);
          do
          {
            __asm
            {
              vmovups ymm0, ymmword ptr [r8]
              vmovups ymm1, ymmword ptr [rcx]
            }
            _R8 += 32;
            _RCX += 32;
            __asm
            {
              vunpckhps xmm2, xmm0, xmm1
              vunpcklps xmm3, xmm0, xmm1
              vextractf128 xmm1, ymm1, 1
              vextractf128 xmm0, ymm0, 1
              vunpckhps xmm4, xmm0, xmm1
              vunpcklps xmm0, xmm0, xmm1
              vinsertf128 ymm2, ymm3, xmm2, 1
              vinsertf128 ymm0, ymm0, xmm4, 1
              vmovups ymmword ptr [rdi+20h], ymm0
              vmovups ymmword ptr [rdi], ymm2
            }
            _RDI += 64LL;
            v24 -= 8;
          }
          while ( v24 != 0 );
          v14 = v42;
          if ( (_DWORD)v18 == 0 )
            goto LABEL_22;
          goto LABEL_20;
        }
        v14 = *((_QWORD *)v43[0] + 19);
      }
    }
    v19 = 0;
    v25 = (_DWORD *)_RDI;
LABEL_20:
    v36 = (int *)(v14 + 4 * v19);
    v37 = (int *)(v15 + 4 * v19);
    v38 = v16 - v19;
    do
    {
      v39 = *v36++;
      *v25 = v39;
      v40 = *v37++;
      v25[1] = v40;
      v25 += 2;
      --v38;
    }
    while ( v38 != 0 );
    goto LABEL_22;
  }
  if ( a4 != 0 )
  {
    *(_DWORD *)(a4 + 32) = 3;
    **(_DWORD **)(a4 + 8) = 2;
    **(_DWORD **)(a4 + 16) = 3;
  }
  return 0;
}
