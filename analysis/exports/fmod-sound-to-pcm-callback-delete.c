__int64 __fastcall fmod_sound_to_pcm_callback_delete(_QWORD *a1, double a2)
{
  FMOD::Sound *v3; // rdi
  __int64 v4; // rdi
  __int64 v5; // rbx

  *a1 = &vtable_FMODSoundToPCMCallback;
  v3 = (FMOD::Sound *)a1[3];
  if ( v3 != nullptr )
  {
    FMOD::Sound::release(v3);
    a1[3] = 0;
  }
  v4 = a1[4];
  if ( v4 != 0 )
  {
    sub_37B800(v4, a2);
    a1[4] = 0;
  }
  v5 = a1[1] + 80LL;
  scePthreadMutexLock(v5);
  *(_QWORD *)(a1[1] + 88LL) = 0;
  scePthreadMutexUnlock(v5);
  return sub_37BF50(a1);
}
