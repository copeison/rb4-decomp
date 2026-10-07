__int64 __fastcall fmod_studio_sound_manager_shutdown_pool(__int64 a1)
{
  unsigned int v1; // r15d
  __int64 v3; // r14
  _QWORD *v4; // rsi
  int v5; // ecx
  __int64 v6; // rax
  __int64 v7; // rcx
  __int64 v8; // rcx
  __int64 v9; // rdx
  __int64 v10; // rsi
  _QWORD *v11; // rbx
  __int64 v12; // r9
  _QWORD *v13; // r9
  _QWORD *v14; // rsi
  bool v15; // zf

  v3 = a1 + 24;
  scePthreadMutexLock(a1 + 24);
  v5 = *(_DWORD *)(a1 + 16) + 1;
  *(_DWORD *)(a1 + 16) = v5;
  if ( *(_QWORD *)(a1 + 48) == *(_DWORD *)(a1 + 8) )
  {
    v6 = *(_QWORD *)(a1 + 64);
    LOBYTE(v1) = 1;
    if ( v6 != 0 )
    {
      v7 = *(_QWORD *)(v6 - 8);
      if ( v7 != 0 )
      {
        v8 = 208 * v7;
        v9 = v6 + v8;
        do
        {
          *(_QWORD *)(v6 + v8 - 208) = &unk_18DCD58;
          v10 = *(_QWORD *)(v6 + v8 - 152);
          if ( v10 != 0 )
          {
            *(_QWORD *)(v6 + v8 - 152) = 0;
            --*(_QWORD *)(v10 + 16);
            v11 = (_QWORD *)(v6 + v8 - 168);
            v12 = *v11;
            *(_QWORD *)(v12 + 8) = *(_QWORD *)(v6 + v8 - 160);
            **(_QWORD **)(v6 + v8 - 160) = v12;
            v13 = (_QWORD *)(v9 - 160);
            v14 = v11;
            *v11 = v11;
            *(_QWORD *)(v6 + v8 - 160) = v11;
          }
          else
          {
            v11 = *(_QWORD **)(v6 + v8 - 168);
            v14 = *(_QWORD **)(v6 + v8 - 160);
            v13 = (_QWORD *)(v6 + v8 - 160);
          }
          v11[1] = v14;
          v9 -= 208;
          v15 = v8 == 208;
          v8 -= 208;
          v4 = (_QWORD *)*v13;
          *(_QWORD *)*v13 = v11;
        }
        while ( !v15 );
      }
      sub_37BF70(v6 - 8, v4);
      v5 = *(_DWORD *)(a1 + 16);
    }
  }
  else
  {
    v1 = 0;
  }
  *(_DWORD *)(a1 + 16) = v5 - 1;
  scePthreadMutexUnlock(v3);
  return v1;
}
