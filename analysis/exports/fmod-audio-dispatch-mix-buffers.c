void __fastcall fmod_audio_dispatch_mix_buffers(void *state, unsigned __int64 mix_sequence)
{
  char *v4; // r15
  int v5; // eax
  _QWORD *v6; // rbx

  v4 = (char *)state + 168;
  scePthreadMutexLock((char *)state + 168);
  v5 = *((_DWORD *)state + 40) + 1;
  *((_DWORD *)state + 40) = v5;
  v6 = *((_QWORD **)state + 22);
  if ( v6 != (_QWORD *)((char *)state + 176) )
  {
    do
    {
      (*(void (__fastcall **)(_QWORD *, _QWORD, unsigned __int64))(*(v6 - 1) + 16LL))(
        v6 - 1,
        *((unsigned int *)state + 74),
        mix_sequence);
      v6 = (_QWORD *)*v6;
    }
    while ( v6 != (_QWORD *)((char *)state + 176) );
    v5 = *((_DWORD *)state + 40);
  }
  *((_DWORD *)state + 40) = v5 - 1;
  scePthreadMutexUnlock(v4);
  audio_dispatch_output_blocks((char *)state + 24, *((_DWORD *)state + 74), mix_sequence);
}
