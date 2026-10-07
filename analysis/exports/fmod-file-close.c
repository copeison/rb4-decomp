__int64 __fastcall fmod_file_close(__int64 a1)
{
  int v2; // r15d
  double v3; // xmm0_8
  int v4; // ebx

  if ( a1 == 0 )
    return 31;
  scePthreadMutexLock(a1 + 24);
  ++*(_DWORD *)(a1 + 16);
  engine_file_close(*(_QWORD *)(a1 + 32));
  --*(_DWORD *)(a1 + 16);
  scePthreadMutexUnlock(a1 + 24);
  scePthreadMutexLock(a1 + 24);
  v2 = *(_DWORD *)(a1 + 16);
  v3 = scePthreadMutexUnlock(a1 + 24);
  if ( v2 > 0 )
  {
    do
    {
      v4 = *(_DWORD *)(a1 + 16);
      *(_DWORD *)(a1 + 16) = v4 - 1;
      v3 = scePthreadMutexUnlock(a1 + 24);
    }
    while ( v4 > 1 );
  }
  scePthreadMutexDestroy(a1 + 24, v3);
  sub_255550(a1);
  sub_37BF50(a1);
  return 0;
}
