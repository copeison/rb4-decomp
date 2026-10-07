// Queues a channel-control/DSP pair in the current deferred-release buffer.
void __fastcall fmod_defer_channel_dsp_release(void *state, void *channel_control, void *dsp)
{
  char *v6; // rbx
  __int64 v7; // r12
  char *v8; // r15
  __int64 v9; // rbx
  char *v10; // rsi
  __int64 v11; // rax
  __int64 v12; // r13
  char *v13; // r15
  __int64 v14; // r15
  __int64 *v15; // rax
  char *v16; // [rsp+8h] [rbp-58h]
  void *v17; // [rsp+10h] [rbp-50h]
  _QWORD *v18; // [rsp+18h] [rbp-48h]
  void *v19; // [rsp+20h] [rbp-40h]
  __int64 *v20; // [rsp+28h] [rbp-38h]
  _QWORD *v21; // [rsp+30h] [rbp-30h]

  v6 = (char *)state + 712;
  scePthreadMutexLock((char *)state + 712);
  ++*((_DWORD *)state + 176);
  if ( *((_QWORD *)state + 35) != 0 )
  {
    v19 = channel_control;
    v7 = 32LL * *((int *)state + 196);
    v8 = *(char **)((char *)state + v7 + 728);
    if ( (unsigned __int64)v8 >= *(_QWORD *)((char *)state + v7 + 736) )
    {
      v17 = dsp;
      v20 = (__int64 *)((char *)state + v7 + 728);
      v16 = v6;
      v9 = 1;
      v10 = *(char **)((char *)state + v7 + 720);
      v18 = (char *)state + v7 + 720;
      if ( v8 != v10 )
        v9 = (v8 - v10) >> 3;
      if ( v9 != 0 )
      {
        v11 = sub_252CF0((char *)state + v7 + 744, 16 * v9, 0);
        v10 = *(char **)((char *)state + v7 + 720);
        v12 = v11;
        v8 = (char *)*v20;
      }
      else
      {
        v12 = 0;
      }
      v13 = (char *)(v8 - v10);
      v21 = (char *)state + v7 + 736;
      memmove(v12, v10, v13);
      *(_QWORD *)&v13[v12] = v19;
      *(_QWORD *)&v13[v12 + 8] = v17;
      v14 = (__int64)&v13[v12 + 16];
      v15 = (__int64 *)((char *)state + v7 + 720);
      if ( *v18 != 0 )
      {
        sub_252D30((char *)state + v7 + 744, *v18, *v21 - *v18);
        v15 = (__int64 *)((char *)state + v7 + 720);
      }
      *v15 = v12;
      *v20 = v14;
      *v21 = 16 * v9 + v12;
      v6 = v16;
    }
    else
    {
      *(_QWORD *)((char *)state + v7 + 728) = v8 + 16;
      *(_QWORD *)v8 = v19;
      *((_QWORD *)v8 + 1) = dsp;
    }
  }
  --*((_DWORD *)state + 176);
  scePthreadMutexUnlock(v6);
}
