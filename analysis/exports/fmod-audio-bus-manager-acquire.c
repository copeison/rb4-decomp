__int64 *__fastcall fmod_audio_bus_manager_acquire(__int64 a1, __int64 a2, __int64 a3)
{
  __int64 v6; // r14
  __int64 v7; // r13
  int v8; // eax
  __int64 v9; // rcx
  __int64 *v10; // rax
  __int64 v11; // rcx
  __int64 *v12; // r15

  v6 = a1 + 24;
  v7 = unk_19C90E8;
  scePthreadMutexLock(a1 + 24);
  v8 = *(_DWORD *)(a1 + 16) + 1;
  *(_DWORD *)(a1 + 16) = v8;
  v9 = *(_QWORD *)(a1 + 48);
  if ( v9 != 0 )
  {
    v10 = *(__int64 **)(a1 + 32);
    if ( a2 != 0 )
      v7 = a2;
    v10[2] = 0;
    *(_QWORD *)(a1 + 48) = v9 - 1;
    v11 = *v10;
    *(_QWORD *)(v11 + 8) = v10[1];
    *(_QWORD *)v10[1] = v11;
    *v10 = (__int64)v10;
    v10[1] = (__int64)v10;
    v10[3] = a3;
    v12 = v10 - 5;
    v10[4] = v7;
    sub_40720((__int64)(v10 - 5));
    v8 = *(_DWORD *)(a1 + 16);
  }
  else
  {
    v12 = nullptr;
  }
  *(_DWORD *)(a1 + 16) = v8 - 1;
  scePthreadMutexUnlock(v6);
  return v12;
}
