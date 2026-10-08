// Destroys renderer settings, callback state, backend resources, default resources, platform configurations, vectors, and mutexes in reverse order.
__int64 __fastcall render_system_destruct(__int64 a1)
{
  __int64 v2; // rdi
  __int64 v3; // rsi
  int v4; // r15d
  double v5; // xmm0_8
  int v6; // ebx
  __int64 v7; // rsi
  __int64 v8; // rsi
  __int64 v9; // rsi
  __int64 v10; // rsi
  __int64 v11; // rsi
  __int64 v12; // rsi
  __int64 v13; // rsi
  __int64 v14; // rsi
  __int64 v15; // rsi
  __int64 v16; // rsi
  __int64 v17; // rsi
  __int64 v18; // rsi
  __int64 v19; // rsi
  __int64 v20; // rsi
  __int64 v21; // rsi
  int v22; // r15d
  double v23; // xmm0_8
  int v24; // ebx

  *(_QWORD *)a1 = &unk_18FF528;
  v2 = *(_QWORD *)(a1 + 296);
  if ( v2 != 0 )
    sub_37BF50(v2);
  *(_QWORD *)(a1 + 296) = 0;
  v3 = *(_QWORD *)(a1 + 3760);
  if ( v3 != 0 )
    sub_252D30(a1 + 3784, v3, *(_QWORD *)(a1 + 3776) - v3);
  scePthreadMutexLock(a1 + 3752);
  v4 = *(_DWORD *)(a1 + 3744);
  v5 = scePthreadMutexUnlock(a1 + 3752);
  if ( v4 > 0 )
  {
    do
    {
      v6 = *(_DWORD *)(a1 + 3744);
      *(_DWORD *)(a1 + 3744) = v6 - 1;
      v5 = scePthreadMutexUnlock(a1 + 3752);
    }
    while ( v6 > 1 );
  }
  scePthreadMutexDestroy(a1 + 3752, v5);
  sub_62ABA0(a1 + 3584);
  nullsub_44(a1 + 3560);
  sub_47EFA0(a1 + 3256);
  sub_63F350(a1 + 2544);
  sub_6BDC20(a1 + 1976);
  v7 = *(_QWORD *)(a1 + 1848);
  if ( v7 != 0 )
    sub_252D30(a1 + 1872, v7, *(_QWORD *)(a1 + 1864) - v7);
  v8 = *(_QWORD *)(a1 + 1720);
  if ( v8 != 0 )
    sub_252D30(a1 + 1744, v8, *(_QWORD *)(a1 + 1736) - v8);
  v9 = *(_QWORD *)(a1 + 1592);
  if ( v9 != 0 )
    sub_252D30(a1 + 1616, v9, *(_QWORD *)(a1 + 1608) - v9);
  v10 = *(_QWORD *)(a1 + 1464);
  if ( v10 != 0 )
    sub_252D30(a1 + 1488, v10, *(_QWORD *)(a1 + 1480) - v10);
  v11 = *(_QWORD *)(a1 + 1336);
  if ( v11 != 0 )
    sub_252D30(a1 + 1360, v11, *(_QWORD *)(a1 + 1352) - v11);
  v12 = *(_QWORD *)(a1 + 1208);
  if ( v12 != 0 )
    sub_252D30(a1 + 1232, v12, *(_QWORD *)(a1 + 1224) - v12);
  v13 = *(_QWORD *)(a1 + 1080);
  if ( v13 != 0 )
    sub_252D30(a1 + 1104, v13, *(_QWORD *)(a1 + 1096) - v13);
  v14 = *(_QWORD *)(a1 + 952);
  if ( v14 != 0 )
    sub_252D30(a1 + 976, v14, *(_QWORD *)(a1 + 968) - v14);
  v15 = *(_QWORD *)(a1 + 824);
  if ( v15 != 0 )
    sub_252D30(a1 + 848, v15, *(_QWORD *)(a1 + 840) - v15);
  v16 = *(_QWORD *)(a1 + 696);
  if ( v16 != 0 )
    sub_252D30(a1 + 720, v16, *(_QWORD *)(a1 + 712) - v16);
  v17 = *(_QWORD *)(a1 + 568);
  if ( v17 != 0 )
    sub_252D30(a1 + 592, v17, *(_QWORD *)(a1 + 584) - v17);
  v18 = *(_QWORD *)(a1 + 440);
  if ( v18 != 0 )
    sub_252D30(a1 + 464, v18, *(_QWORD *)(a1 + 456) - v18);
  v19 = *(_QWORD *)(a1 + 312);
  if ( v19 != 0 )
    sub_252D30(a1 + 336, v19, *(_QWORD *)(a1 + 328) - v19);
  v20 = *(_QWORD *)(a1 + 128);
  if ( v20 != 0 )
    sub_252D30(a1 + 152, v20, *(_QWORD *)(a1 + 144) - v20);
  v21 = *(_QWORD *)(a1 + 72);
  if ( v21 != 0 )
    sub_252D30(a1 + 96, v21, *(_QWORD *)(a1 + 88) - v21);
  scePthreadMutexLock(a1 + 16);
  v22 = *(_DWORD *)(a1 + 8);
  v23 = scePthreadMutexUnlock(a1 + 16);
  if ( v22 > 0 )
  {
    do
    {
      v24 = *(_DWORD *)(a1 + 8);
      *(_DWORD *)(a1 + 8) = v24 - 1;
      v23 = scePthreadMutexUnlock(a1 + 16);
    }
    while ( v24 > 1 );
  }
  return scePthreadMutexDestroy(a1 + 16, v23);
}
