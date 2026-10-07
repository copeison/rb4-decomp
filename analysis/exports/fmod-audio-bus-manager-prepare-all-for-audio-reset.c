double __fastcall fmod_audio_bus_manager_prepare_all_for_audio_reset(__int64 a1)
{
  __int64 v2; // r14
  int v3; // eax
  __int64 v4; // rbx
  __int64 v5; // r12

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
      (*(void (__fastcall **)(__int64))(*(_QWORD *)(*(_QWORD *)(a1 + 64) + v4) + 16LL))(v4 + *(_QWORD *)(a1 + 64));
      ++v5;
      v4 += 448;
    }
    while ( v5 < *(int *)(a1 + 8) );
    v3 = *(_DWORD *)(a1 + 16);
  }
  *(_DWORD *)(a1 + 16) = v3 - 1;
  return scePthreadMutexUnlock(v2);
}
