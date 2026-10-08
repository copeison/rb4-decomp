// Accounts for a skipped render frame under the render-system lock.
double __fastcall render_system_skip_frame(__int64 a1)
{
  __int64 v2; // r14
  int v3; // eax

  v2 = a1 + 16;
  scePthreadMutexLock(a1 + 16);
  v3 = *(_DWORD *)(a1 + 8);
  ++*(_QWORD *)(a1 + 160);
  *(_DWORD *)(a1 + 8) = v3;
  return scePthreadMutexUnlock(v2);
}
