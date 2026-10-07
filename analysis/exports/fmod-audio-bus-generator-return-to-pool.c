double __fastcall fmod_audio_bus_generator_return_to_pool(__int64 a1)
{
  __int64 v2; // r15
  __int64 v3; // rax
  _QWORD *v4; // rcx

  v2 = *(_QWORD *)(a1 + 16);
  scePthreadMutexLock(v2 + 24);
  ++*(_DWORD *)(v2 + 16);
  *(_BYTE *)(a1 + 39) &= ~0x80u;
  *(_QWORD *)(a1 + 64) = 0;
  v3 = v2 + 32;
  if ( *(_QWORD *)(a1 + 56) != v2 + 32 )
  {
    *(_QWORD *)(a1 + 56) = v3;
    ++*(_QWORD *)(v2 + 48);
    v4 = *(_QWORD **)(v2 + 40);
    *(_QWORD *)(a1 + 48) = v4;
    *(_QWORD *)(a1 + 40) = v3;
    *v4 = a1 + 40;
    *(_QWORD *)(v2 + 40) = a1 + 40;
  }
  --*(_DWORD *)(v2 + 16);
  return scePthreadMutexUnlock(v2 + 24);
}
