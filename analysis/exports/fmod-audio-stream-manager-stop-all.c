double __fastcall fmod_audio_stream_manager_stop_all(__int64 a1)
{
  __int64 v2; // r14
  int v3; // eax
  __int64 v4; // r12
  __int64 v5; // rbx

  v2 = a1 + 24;
  scePthreadMutexLock(a1 + 24);
  v3 = *(_DWORD *)(a1 + 16) + 1;
  *(_DWORD *)(a1 + 16) = v3;
  if ( *(int *)(a1 + 8) > 0 )
  {
    v4 = 0;
    v5 = 0;
    do
    {
      sub_406D0(v4 + *(_QWORD *)(a1 + 64));
      ++v5;
      v4 += 288;
    }
    while ( v5 < *(int *)(a1 + 8) );
    v3 = *(_DWORD *)(a1 + 16);
  }
  *(_DWORD *)(a1 + 16) = v3 - 1;
  return scePthreadMutexUnlock(v2);
}
