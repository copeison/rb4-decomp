// Clears DSP user data while holding the runtime owner lock when available.
void __fastcall audio_clip_fmod_clear_dsp_user_data(void *clip)
{
  __int64 v2; // rdi
  __int64 v3; // r15

  v2 = *((_QWORD *)clip + 15);
  if ( v2 != 0 && (sub_57560(v2, (char *)clip + 80), (v3 = *((_QWORD *)clip + 15)) != 0) )
  {
    scePthreadMutexLock(v3 + 40);
    ++*(_DWORD *)(v3 + 32);
    FMOD::DSP::setUserData(*((FMOD::DSP **)clip + 49), nullptr);
    --*(_DWORD *)(v3 + 32);
    scePthreadMutexUnlock(v3 + 40);
  }
  else
  {
    FMOD::DSP::setUserData(*((FMOD::DSP **)clip + 49), nullptr);
  }
}
