__int64 __fastcall fmod_audio_stream_manager_shutdown_pool(__int64 a1)
{
  __int64 v1; // r12
  __int64 v2; // r15
  __int64 v3; // r14
  __int64 v4; // rsi
  int v5; // eax
  __int64 v6; // rbx
  __int64 v7; // rax
  __int64 v8; // r15
  __int64 v9; // rax
  __int64 v10; // rbx
  __int64 v11; // rax
  _QWORD *v12; // rax
  __int64 v13; // rcx
  _QWORD **v14; // rcx
  _QWORD *v15; // rdx
  __int64 v17; // [rsp+8h] [rbp-48h]
  __int64 v18; // [rsp+18h] [rbp-38h]
  __int64 v19; // [rsp+20h] [rbp-30h]

  v2 = a1;
  v3 = a1 + 24;
  scePthreadMutexLock(a1 + 24);
  v5 = *(_DWORD *)(v2 + 16) + 1;
  *(_DWORD *)(v2 + 16) = v5;
  if ( *(_QWORD *)(v2 + 48) == *(_DWORD *)(v2 + 8) )
  {
    v6 = *(_QWORD *)(v2 + 64);
    LOBYTE(v1) = 1;
    if ( v6 != 0 )
    {
      v18 = v2;
      v19 = v6 - 8;
      v7 = *(_QWORD *)(v6 - 8);
      if ( v7 != 0 )
      {
        v8 = 0;
        v9 = 288 * v7;
        v10 = v9 + v6;
        v17 = v9;
        v1 = v10;
        do
        {
          *(_QWORD *)(v10 + v8 - 288) = &unk_18F0418;
          sub_406D0(v10 + v8 - 288);
          *(_QWORD *)(v10 + v8 - 288) = &unk_18DCD58;
          v11 = *(_QWORD *)(v10 + v8 - 232);
          if ( v11 != 0 )
          {
            *(_QWORD *)(v10 + v8 - 232) = 0;
            --*(_QWORD *)(v11 + 16);
            v12 = (_QWORD *)(v10 + v8 - 248);
            v13 = *v12;
            *(_QWORD *)(v13 + 8) = *(_QWORD *)(v10 + v8 - 240);
            **(_QWORD **)(v10 + v8 - 240) = v13;
            v14 = (_QWORD **)(v1 - 240);
            v15 = v12;
            *v12 = v12;
            *(_QWORD *)(v10 + v8 - 240) = v12;
          }
          else
          {
            v12 = *(_QWORD **)(v10 + v8 - 248);
            v15 = *(_QWORD **)(v10 + v8 - 240);
            v14 = (_QWORD **)(v10 + v8 - 240);
          }
          v12[1] = v15;
          v8 -= 288;
          v1 -= 288;
          **v14 = v12;
        }
        while ( v8 + v17 != 0 );
      }
      sub_37BF70(v19, v4);
      v2 = v18;
      LOBYTE(v1) = 1;
      v5 = *(_DWORD *)(v18 + 16);
    }
  }
  else
  {
    LODWORD(v1) = 0;
  }
  *(_DWORD *)(v2 + 16) = v5 - 1;
  scePthreadMutexUnlock(v3);
  return (unsigned int)v1;
}
