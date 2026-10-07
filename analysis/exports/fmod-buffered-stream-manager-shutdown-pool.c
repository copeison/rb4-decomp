__int64 __fastcall fmod_buffered_stream_manager_shutdown_pool(__int64 a1)
{
  unsigned int v1; // r15d
  __int64 v3; // r14
  __int64 v4; // rsi
  int v5; // eax
  __int64 v6; // rbx
  __int64 v7; // rax
  __int64 v8; // r12

  v3 = a1 + 24;
  scePthreadMutexLock(a1 + 24);
  v5 = *(_DWORD *)(a1 + 16) + 1;
  *(_DWORD *)(a1 + 16) = v5;
  if ( *(_QWORD *)(a1 + 48) == *(_DWORD *)(a1 + 8) )
  {
    v6 = *(_QWORD *)(a1 + 64);
    LOBYTE(v1) = 1;
    if ( v6 != 0 )
    {
      v7 = *(_QWORD *)(v6 - 8);
      if ( v7 != 0 )
      {
        v8 = 568 * v7;
        do
        {
          fmod_buffered_stream_generator_destruct((_QWORD *)(v6 + v8 - 568));
          v8 -= 568;
        }
        while ( v8 != 0 );
      }
      sub_37BF70(v6 - 8, v4);
      v5 = *(_DWORD *)(a1 + 16);
    }
  }
  else
  {
    v1 = 0;
  }
  *(_DWORD *)(a1 + 16) = v5 - 1;
  scePthreadMutexUnlock(v3);
  return v1;
}
