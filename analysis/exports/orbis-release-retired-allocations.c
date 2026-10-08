// Releases deferred GPU allocations whose retirement frame is at least two frames behind.
double __fastcall orbis_release_retired_allocations(__int64 a1, double result)
{
  unsigned __int64 v3; // r13
  __int64 v4; // r14
  double v5; // xmm0_8
  int v6; // eax
  _QWORD *v7; // r15
  unsigned __int64 v8; // r13
  _QWORD *v9; // r14
  __int64 *v10; // rsi
  __int64 v11; // rax

  v3 = *(_QWORD *)(a1 + 160);
  if ( v3 >= 2 )
  {
    v4 = a1 + 4304;
    scePthreadMutexLock(a1 + 4304);
    v6 = *(_DWORD *)(a1 + 4296) + 1;
    *(_DWORD *)(a1 + 4296) = v6;
    v7 = *(_QWORD **)(a1 + 4312);
    if ( v7 != (_QWORD *)(a1 + 4312) )
    {
      v8 = v3 - 2;
      v9 = *(_QWORD **)(a1 + 4312);
      do
      {
        v7 = (_QWORD *)*v7;
        if ( v9[3] <= v8 )
        {
          sub_37B800(v9[2], v5);
          v10 = *(__int64 **)(*v9 + 8LL);
          v11 = *v10;
          *(_QWORD *)(v11 + 8) = v10[1];
          *(_QWORD *)v10[1] = v11;
          sub_252D30(a1 + 4336, v10, 32);
          --*(_QWORD *)(a1 + 4328);
        }
        v9 = v7;
      }
      while ( v7 != (_QWORD *)(a1 + 4312) );
      v6 = *(_DWORD *)(a1 + 4296);
      v4 = a1 + 4304;
    }
    *(_DWORD *)(a1 + 4296) = v6 - 1;
    return scePthreadMutexUnlock(v4);
  }
  return result;
}
