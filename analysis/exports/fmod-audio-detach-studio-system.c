// Stops callbacks, clears deferred releases, and detaches FMOD.
void __fastcall fmod_audio_detach_studio_system(void *state)
{
  char *v2; // rbx
  __int64 v3; // rdi
  int v4; // eax

  _R14 = state;
  v2 = (char *)state + 680;
  while ( (unsigned int)sem_wait(v2) != 0 )
    _error(v3);
  _R14[308] = 1;
  scePthreadMutexLock(_R14 + 712);
  v4 = *((_DWORD *)_R14 + 176);
  *((_QWORD *)_R14 + 91) = *((_QWORD *)_R14 + 90);
  *((_QWORD *)_R14 + 95) = *((_QWORD *)_R14 + 94);
  *((_DWORD *)_R14 + 176) = v4;
  *(double *)&_XMM0 = scePthreadMutexUnlock(_R14 + 712);
  __asm { vxorps  xmm0, xmm0, xmm0 }
  __asm { vmovups xmmword ptr [r14+118h], xmm0 }
  sem_post(v2);
}
