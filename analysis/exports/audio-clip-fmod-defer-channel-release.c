// Detaches DSP user data and queues the low-level channel and DSP for deferred release.
void __fastcall audio_clip_fmod_defer_channel_release(void *clip)
{
  __int64 v3; // rdi
  __int64 v4; // r12

  if ( *((_QWORD *)clip + 50) != 0 )
  {
    *((_DWORD *)clip + 7) = 6;
    _R15 = (char *)clip + 400;
    v3 = *((_QWORD *)clip + 15);
    if ( v3 != 0 && (sub_57560(v3, (char *)clip + 80), (v4 = *((_QWORD *)clip + 15)) != 0) )
    {
      scePthreadMutexLock(v4 + 40);
      ++*(_DWORD *)(v4 + 32);
      FMOD::DSP::setUserData(*((FMOD::DSP **)clip + 49), nullptr);
      --*(_DWORD *)(v4 + 32);
      scePthreadMutexUnlock(v4 + 40);
    }
    else
    {
      FMOD::DSP::setUserData(*((FMOD::DSP **)clip + 49), nullptr);
    }
    fmod_defer_channel_dsp_release(*((void **)clip + 9), *((void **)clip + 50), *((void **)clip + 49));
    __asm
    {
      vxorps  xmm0, xmm0, xmm0
      vmovups xmmword ptr [r15], xmm0
    }
    *((_QWORD *)clip + 52) = 0;
    *((_QWORD *)clip + 49) = 0;
    *((_DWORD *)clip + 7) = 5;
  }
}
