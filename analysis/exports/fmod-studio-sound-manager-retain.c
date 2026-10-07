__int64 __fastcall fmod_studio_sound_manager_retain(__int64 a1, int a2, int a3)
{
  __int64 v6; // r14
  int v7; // eax
  __int64 v8; // rcx
  __int64 v9; // r15
  __int64 v10; // r15

  v6 = a1 + 24;
  scePthreadMutexLock(a1 + 24);
  v7 = *(_DWORD *)(a1 + 16) + 1;
  *(_DWORD *)(a1 + 16) = v7;
  if ( *(_DWORD *)(a1 + 8) >= a3 && (v8 = *(_QWORD *)(a1 + 64), v9 = 208LL * a3, *(_DWORD *)(v8 + v9 + 36) == a2) )
  {
    _InterlockedIncrement((volatile signed __int32 *)(v8 + v9 + 32));
    v7 = *(_DWORD *)(a1 + 16);
    v10 = *(_QWORD *)(a1 + 64) + v9;
  }
  else
  {
    v10 = 0;
  }
  *(_DWORD *)(a1 + 16) = v7 - 1;
  scePthreadMutexUnlock(v6);
  return v10;
}
