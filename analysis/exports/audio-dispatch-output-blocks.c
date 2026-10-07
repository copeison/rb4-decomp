void __fastcall audio_dispatch_output_blocks(void *dispatcher, unsigned int buffer_length, unsigned int mix_sequence)
{
  char *v6; // r14
  _QWORD *v7; // r10
  int v8; // r8d
  char *v9; // r9
  char **v10; // rsi
  __int64 v11; // rdx
  __int64 v12; // rbx
  char *v13; // rdi
  char *v14; // rax
  char *v15; // rcx
  _QWORD *v18; // rsi
  int **v20; // r14
  unsigned int v21; // ebx
  int *v22; // r15
  char *v25; // [rsp+8h] [rbp-48h]
  _QWORD *v27; // [rsp+18h] [rbp-38h]

  v25 = (char *)dispatcher + 40;
  scePthreadMutexLock((char *)dispatcher + 40);
  ++*((_DWORD *)dispatcher + 8);
  v6 = (char *)dispatcher + 80;
  scePthreadMutexLock((char *)dispatcher + 80);
  v7 = dispatcher;
  v8 = *((_DWORD *)dispatcher + 18);
  v9 = (char *)dispatcher + 88;
  *((_DWORD *)dispatcher + 18) = v8 + 1;
  v10 = *((char ***)dispatcher + 11);
  if ( v10 != (char **)((char *)dispatcher + 88) )
  {
    v11 = *((_QWORD *)dispatcher + 8);
    v12 = *((_QWORD *)dispatcher + 13);
    v13 = (char *)(v7 + 6);
    do
    {
      v14 = *v10;
      --v12;
      ++v11;
      *((_QWORD *)v14 + 1) = v10[1];
      *(_QWORD *)v10[1] = v14;
      *v10 = (char *)v10;
      v10[1] = (char *)v10;
      v10[2] = v13;
      v15 = (char *)v7[7];
      v10[1] = v15;
      *v10 = v13;
      *(_QWORD *)v15 = v10;
      v7[7] = v10;
      v10 = (char **)v14;
    }
    while ( v14 != v9 );
    v7[13] = v12;
    v7[8] = v11;
  }
  *((_DWORD *)v7 + 18) = v8;
  v27 = v7;
  *(double *)&_XMM0 = scePthreadMutexUnlock(v6);
  v18 = v27;
  if ( buffer_length != 0 )
  {
    __asm { vcvtsi2ss xmm0, xmm0, dword ptr [rsi+70h] }
    v20 = (int **)(v27 + 6);
    v21 = 0;
    __asm { vmovss  [rbp+var_3C], xmm0 }
    while ( 1 )
    {
      v22 = *v20;
      if ( *v20 != (int *)v20 )
      {
        do
        {
          __asm { vmovss  xmm0, [rbp+var_3C] }
          (*(void (__fastcall **)(_QWORD *, __int64, _QWORD, _QWORD, bool, double))(*((_QWORD *)v22 - 2) + 16LL))(
            (_QWORD *)v22 - 2,
            128,
            mix_sequence,
            v21,
            buffer_length == 128,
            *(double *)&_XMM0);
          v22 = *(int **)v22;
        }
        while ( v22 != (int *)v20 );
        v22 = *v20;
      }
      v18 = v27;
      *((_DWORD *)v27 + 29) = mix_sequence;
      *((_DWORD *)v27 + 30) = v21;
      *((_BYTE *)v27 + 124) = buffer_length == 128;
      if ( v22 != (int *)v20 )
        break;
      while ( v22 != (int *)v20 )
      {
LABEL_16:
        if ( *(v22 - 2) <= 0 && _InterlockedExchangeAdd(v22 - 2, 1u) == 0 )
        {
          __asm { vcvtsi2ss xmm0, xmm1, dword ptr [rsi+70h] }
          (*(void (__fastcall **)(_QWORD *, __int64, _QWORD, _QWORD, _QWORD, double))(*((_QWORD *)v22 - 2) + 24LL))(
            (_QWORD *)v22 - 2,
            128,
            *((unsigned int *)v18 + 29),
            *((unsigned int *)v18 + 30),
            *((unsigned __int8 *)v18 + 124),
            *(double *)&_XMM0);
          v18 = v27;
        }
        v22 = *(int **)v22;
      }
LABEL_19:
      ++v21;
      buffer_length -= 128;
      if ( buffer_length == 0 )
        goto LABEL_20;
    }
    do
    {
      _InterlockedExchange(v22 - 2, 0);
      v22 = *(int **)v22;
    }
    while ( v22 != (int *)v20 );
    v22 = *v20;
    if ( *v20 == (int *)v20 )
      goto LABEL_19;
    goto LABEL_16;
  }
LABEL_20:
  --*((_DWORD *)v18 + 8);
  scePthreadMutexUnlock(v25);
}
