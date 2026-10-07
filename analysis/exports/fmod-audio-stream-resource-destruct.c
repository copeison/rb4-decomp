__int64 __fastcall fmod_audio_stream_resource_destruct(__int64 a1)
{
  FMOD::Sound *v2; // rdi
  int v3; // r15d
  double v4; // xmm0_8
  int v5; // ebx

  *(_QWORD *)a1 = &vtable_FmodAudioStreamResource;
  v2 = *(FMOD::Sound **)(a1 + 96);
  if ( v2 != nullptr )
  {
    FMOD::Sound::release(v2);
    *(_QWORD *)(a1 + 96) = 0;
  }
  if ( *(_QWORD *)(a1 + 48) != 19246190 )
    fmod_audio_stream_resource_unregister(a1);
  scePthreadMutexLock(a1 + 80);
  v3 = *(_DWORD *)(a1 + 72);
  v4 = scePthreadMutexUnlock(a1 + 80);
  if ( v3 > 0 )
  {
    do
    {
      v5 = *(_DWORD *)(a1 + 72);
      *(_DWORD *)(a1 + 72) = v5 - 1;
      v4 = scePthreadMutexUnlock(a1 + 80);
    }
    while ( v5 > 1 );
  }
  scePthreadMutexDestroy(a1 + 80, v4);
  return sub_1ADD70(a1);
}
