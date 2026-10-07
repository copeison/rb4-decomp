__int64 __fastcall fmod_audio_stream_manager_retain(__int64 a1, int a2, int a3)
{
  __int64 v6; // r14
  int v7; // eax
  __int64 v8; // rcx
  __int64 v9; // r15

  v6 = a1 + 24;
  scePthreadMutexLock(a1 + 24);
  v7 = *(_DWORD *)(a1 + 16) + 1;
  *(_DWORD *)(a1 + 16) = v7;
  if ( *(_DWORD *)(a1 + 8) >= a3 && *(_DWORD *)((v8 = *(_QWORD *)(a1 + 64)) + 288LL * a3 + 36) == a2 )
  {
    _InterlockedIncrement((volatile signed __int32 *)(v8 + 288LL * a3 + 32));
    v7 = *(_DWORD *)(a1 + 16);
    v9 = *(_QWORD *)(a1 + 64) + 288LL * a3;
  }
  else
  {
    v9 = 0;
  }
  *(_DWORD *)(a1 + 16) = v7 - 1;
  scePthreadMutexUnlock(v6);
  return v9;
}
