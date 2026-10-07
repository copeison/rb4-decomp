__int64 __fastcall fmod_file_open(__int64 a1, __int64 a2, __int64 *a3)
{
  __int64 v6; // r13
  unsigned int v9; // r15d
  int v10; // r14d
  double v11; // xmm0_8
  int v12; // ebx
  _BYTE v15[8]; // [rsp+8h] [rbp-38h] BYREF
  __int64 v16; // [rsp+10h] [rbp-30h]

  v16 = 0x6365786562696C2FLL;
  v6 = sub_37BF40(40);
  __asm { vxorps  ymm0, ymm0, ymm0 }
  _RDI = v6;
  __asm { vmovups ymmword ptr [r13+0], ymm0 }
  *(_QWORD *)(v6 + 32) = 0;
  sub_255080(v6);
  *(_DWORD *)(v6 + 16) = 0;
  scePthreadMutexattrInit(v15);
  scePthreadMutexattrSettype(v15, 2);
  scePthreadMutexInit(v6 + 24, v15, "hx crit sec");
  scePthreadMutexattrDestroy(v15);
  v9 = fmod_file_handle_open(v6, a1, a2);
  if ( v9 != 0 )
  {
    scePthreadMutexLock(v6 + 24);
    v10 = *(_DWORD *)(v6 + 16);
    v11 = scePthreadMutexUnlock(v6 + 24);
    if ( v10 > 0 )
    {
      do
      {
        v12 = *(_DWORD *)(v6 + 16);
        *(_DWORD *)(v6 + 16) = v12 - 1;
        v11 = scePthreadMutexUnlock(v6 + 24);
      }
      while ( v12 > 1 );
    }
    scePthreadMutexDestroy(v6 + 24, v11);
    sub_255550(v6);
    sub_37BF50(v6);
  }
  else
  {
    *a3 = v6;
  }
  return v9;
}
