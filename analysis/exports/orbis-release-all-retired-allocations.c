// Releases and removes every allocation in the deferred-retirement queue.
double __fastcall orbis_release_all_retired_allocations(__int64 a1)
{
  __int64 v1; // rax
  __int64 v3; // r14
  double v4; // xmm0_8
  int v5; // eax
  _QWORD *v6; // rbx
  _QWORD *v7; // r14
  __int64 *v8; // rsi
  __int64 v9; // rax
  __int64 savedregs; // [rsp+28h] [rbp+0h]

  savedregs = v1;
  v3 = a1 + 4304;
  scePthreadMutexLock(a1 + 4304);
  v5 = *(_DWORD *)(a1 + 4296) + 1;
  *(_DWORD *)(a1 + 4296) = v5;
  v6 = *(_QWORD **)(a1 + 4312);
  if ( v6 != (_QWORD *)(a1 + 4312) )
  {
    *(&savedregs - 6) = v3;
    v7 = v6;
    do
    {
      v6 = (_QWORD *)*v6;
      sub_37B800(v7[2], v4);
      v8 = *(__int64 **)(*v7 + 8LL);
      v9 = *v8;
      *(_QWORD *)(v9 + 8) = v8[1];
      *(_QWORD *)v8[1] = v9;
      sub_252D30(a1 + 4336, v8, 32);
      --*(_QWORD *)(a1 + 4328);
      v7 = v6;
    }
    while ( v6 != (_QWORD *)(a1 + 4312) );
    v5 = *(_DWORD *)(a1 + 4296);
    v3 = *(&savedregs - 6);
  }
  *(_DWORD *)(a1 + 4296) = v5 - 1;
  return scePthreadMutexUnlock(v3);
}
