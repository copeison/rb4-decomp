// Appends a consumer to the mutex-protected mix-dispatch list.
void __fastcall audio_register_mix_consumer(void *state, void *consumer)
{
  char *v4; // r15
  int v5; // eax
  _QWORD *v6; // rdx

  v4 = (char *)state + 168;
  scePthreadMutexLock((char *)state + 168);
  v5 = *((_DWORD *)state + 40);
  *((_QWORD *)consumer + 3) = (char *)state + 176;
  ++*((_QWORD *)state + 24);
  v6 = *((_QWORD **)state + 23);
  *((_QWORD *)consumer + 2) = v6;
  *((_QWORD *)consumer + 1) = (char *)state + 176;
  *v6 = (char *)consumer + 8;
  *((_QWORD *)state + 23) = (char *)consumer + 8;
  *((_DWORD *)state + 40) = v5;
  scePthreadMutexUnlock(v4);
}
