__int64 __fastcall fmod_audio_bus_manager_shutdown_pool(__int64 a1)
{
  unsigned int v1; // r15d
  __int64 v3; // r14
  __int64 v4; // rsi
  __m128 v5; // xmm0
  int v6; // eax
  __int64 v7; // rbx
  __int64 v8; // rax
  __int64 v9; // r12

  v3 = a1 + 24;
  scePthreadMutexLock(a1 + 24);
  v6 = *(_DWORD *)(a1 + 16) + 1;
  *(_DWORD *)(a1 + 16) = v6;
  if ( *(_QWORD *)(a1 + 48) == *(_DWORD *)(a1 + 8) )
  {
    v7 = *(_QWORD *)(a1 + 64);
    LOBYTE(v1) = 1;
    if ( v7 != 0 )
    {
      v8 = *(_QWORD *)(v7 - 8);
      if ( v8 != 0 )
      {
        v9 = 448 * v8;
        do
        {
          sub_E0A60(v7 + v9 - 448, v5);
          v9 -= 448;
        }
        while ( v9 != 0 );
      }
      sub_37BF70(v7 - 8, v4);
      v6 = *(_DWORD *)(a1 + 16);
    }
  }
  else
  {
    v1 = 0;
  }
  *(_DWORD *)(a1 + 16) = v6 - 1;
  scePthreadMutexUnlock(v3);
  return v1;
}
