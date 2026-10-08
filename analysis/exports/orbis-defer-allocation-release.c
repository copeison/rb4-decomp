// Enqueues a non-null GPU allocation with the current frame index for deferred release.
double __fastcall orbis_defer_allocation_release(__int64 a1, __int64 a2)
{
  __int64 v4; // r12
  _QWORD *v5; // rax
  double result; // xmm0_8

  if ( a2 != 0 )
  {
    scePthreadMutexLock(a1 + 4304);
    ++*(_DWORD *)(a1 + 4296);
    v4 = *(_QWORD *)(g_orbis_render_system + 160LL);
    v5 = (_QWORD *)sub_252CF0(a1 + 4336, 32, 0);
    v5[2] = a2;
    v5[3] = v4;
    *v5 = a1 + 4312;
    v5[1] = *(_QWORD *)(a1 + 4320);
    **(_QWORD **)(a1 + 4320) = v5;
    *(_QWORD *)(a1 + 4320) = v5;
    ++*(_QWORD *)(a1 + 4328);
    --*(_DWORD *)(a1 + 4296);
    return scePthreadMutexUnlock(a1 + 4304);
  }
  return result;
}
