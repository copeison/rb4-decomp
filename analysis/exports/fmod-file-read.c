__int64 __fastcall fmod_file_read(__int64 a1, __int64 a2, unsigned int a3, unsigned int *a4)
{
  unsigned int v8; // eax
  unsigned int v9; // r14d

  if ( a1 != 0 )
  {
    scePthreadMutexLock(a1 + 24);
    ++*(_DWORD *)(a1 + 16);
    v8 = engine_file_read(*(_QWORD *)(a1 + 32), a2, a3);
    *a4 = v8;
    --*(_DWORD *)(a1 + 16);
    v9 = v8 < a3 ? 0x10 : 0;
    scePthreadMutexUnlock(a1 + 24);
  }
  else
  {
    return 31;
  }
  return v9;
}
