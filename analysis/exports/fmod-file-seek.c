__int64 __fastcall fmod_file_seek(__int64 a1, unsigned int a2)
{
  __int64 v4; // rsi
  unsigned int v5; // r14d

  if ( a1 != 0 )
  {
    scePthreadMutexLock(a1 + 24);
    ++*(_DWORD *)(a1 + 16);
    v4 = a2;
    v5 = 0;
    engine_file_seek(*(_QWORD *)(a1 + 32), v4, 0);
    --*(_DWORD *)(a1 + 16);
    scePthreadMutexUnlock(a1 + 24);
  }
  else
  {
    return 31;
  }
  return v5;
}
