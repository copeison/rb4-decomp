// Destroys Orbis-specific command, synchronization, profiling, and condition state before the base renderer.
__int64 __fastcall orbis_render_system_destruct(__int64 a1)
{
  _QWORD *v2; // r15
  _QWORD *v3; // rsi
  _QWORD *v4; // rbx
  int v5; // ebx
  double v6; // xmm0_8
  int v7; // ebx
  int v8; // ebx
  double v9; // xmm0_8
  int v10; // ebx

  v2 = (_QWORD *)(a1 + 4312);
  *(_QWORD *)a1 = &unk_195ED70;
  g_orbis_render_system = 0;
  v3 = *(_QWORD **)(a1 + 4312);
  if ( v3 != (_QWORD *)(a1 + 4312) )
  {
    do
    {
      v4 = (_QWORD *)*v3;
      sub_252D30(a1 + 4336, v3, 32);
      v3 = v4;
    }
    while ( v4 != v2 );
  }
  scePthreadMutexLock(a1 + 4304);
  v5 = *(_DWORD *)(a1 + 4296);
  v6 = scePthreadMutexUnlock(a1 + 4304);
  if ( v5 > 0 )
  {
    do
    {
      v7 = *(_DWORD *)(a1 + 4296);
      *(_DWORD *)(a1 + 4296) = v7 - 1;
      v6 = scePthreadMutexUnlock(a1 + 4304);
    }
    while ( v7 > 1 );
  }
  scePthreadMutexDestroy(a1 + 4304, v6);
  scePthreadMutexLock(a1 + 4288);
  v8 = *(_DWORD *)(a1 + 4280);
  v9 = scePthreadMutexUnlock(a1 + 4288);
  if ( v8 > 0 )
  {
    do
    {
      v10 = *(_DWORD *)(a1 + 4280);
      *(_DWORD *)(a1 + 4280) = v10 - 1;
      v9 = scePthreadMutexUnlock(a1 + 4288);
    }
    while ( v10 > 1 );
  }
  scePthreadMutexDestroy(a1 + 4288, v9);
  sub_25C3C0(a1 + 4152);
  if ( *(_QWORD *)(a1 + 3824) != 0 )
  {
    scePthreadCondDestroy(a1 + 3832);
    *(_QWORD *)(a1 + 3824) = 0;
  }
  return render_system_destruct(a1);
}
