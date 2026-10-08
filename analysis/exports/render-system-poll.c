// Polls the render system's frame owner under its synchronization lock.
__int64 __fastcall render_system_poll(__int64 a1)
{
  unsigned int v1; // r15d
  __int64 v3; // r14

  v3 = a1 + 16;
  scePthreadMutexLock(a1 + 16);
  ++*(_DWORD *)(a1 + 8);
  (*(void (__fastcall **)(_QWORD))(**(_QWORD **)(a1 + 112) + 32LL))(*(_QWORD *)(a1 + 112));
  LOBYTE(v1) = g_render_system != 0;
  --*(_DWORD *)(a1 + 8);
  scePthreadMutexUnlock(v3);
  return v1;
}
