__int64 __fastcall fmod_dialog_manager_shutdown_pool(__int64 a1)
{
  void *v1; // r12
  __int64 v2; // r15
  __int64 v3; // r14
  __int64 v4; // rsi
  int v5; // eax
  __int64 v6; // rbx
  __int64 v7; // rax
  __int64 v8; // rax
  __int64 v9; // r15
  __int64 v10; // rbx
  __int64 v11; // r14
  __int64 v12; // rdi
  __int64 v13; // rax
  _QWORD *v14; // rax
  __int64 v15; // rcx
  _QWORD **v16; // rcx
  _QWORD *v17; // rdx
  __int64 v18; // rdi
  __int64 v19; // rax
  _QWORD *v20; // rax
  __int64 v21; // rcx
  _QWORD **v22; // rcx
  _QWORD *v23; // rdx
  __int64 v25; // [rsp+0h] [rbp-50h]
  __int64 v26; // [rsp+10h] [rbp-40h]
  __int64 v27; // [rsp+18h] [rbp-38h]
  __int64 v28; // [rsp+20h] [rbp-30h]

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
      v27 = v2;
      v26 = v3;
      v28 = v6 - 16;
      v7 = *(_QWORD *)(v6 - 8);
      if ( v7 != 0 )
      {
        v8 = 416 * v7;
        v9 = 0;
        v10 = v8 + v6;
        v25 = v8;
        v11 = v10;
        v1 = &unk_199FFC8;
        do
        {
          *(_QWORD *)(v10 + v9 - 416) = &vtable_FmodDialogGenerator;
          v12 = *(_QWORD *)(v10 + v9 - 32);
          if ( v12 != 0 )
          {
            (*(void (__fastcall **)(__int64, bool))(*(_QWORD *)v12 + 32LL))(v12, v10 + v9 - 64 != v12);
            *(_QWORD *)(v10 + v9 - 32) = 0;
          }
          *(_QWORD *)(v10 + v9 - 280) = &unk_18DCD58;
          v13 = *(_QWORD *)(v10 + v9 - 224);
          if ( v13 != 0 )
          {
            *(_QWORD *)(v10 + v9 - 224) = 0;
            --*(_QWORD *)(v13 + 16);
            v14 = (_QWORD *)(v10 + v9 - 240);
            v15 = *v14;
            *(_QWORD *)(v15 + 8) = *(_QWORD *)(v10 + v9 - 232);
            **(_QWORD **)(v10 + v9 - 232) = v15;
            v16 = (_QWORD **)(v11 - 232);
            v17 = v14;
            *v14 = v14;
            *(_QWORD *)(v10 + v9 - 232) = v14;
          }
          else
          {
            v14 = *(_QWORD **)(v10 + v9 - 240);
            v17 = *(_QWORD **)(v10 + v9 - 232);
            v16 = (_QWORD **)(v10 + v9 - 232);
          }
          v14[1] = v17;
          **v16 = v14;
          *(_QWORD *)(v10 + v9 - 416) = &unk_199FFC8;
          v18 = *(_QWORD *)(v10 + v9 - 304);
          if ( v18 != 0 )
          {
            (*(void (__fastcall **)(__int64, bool))(*(_QWORD *)v18 + 32LL))(v18, v10 + v9 - 336 != v18);
            *(_QWORD *)(v10 + v9 - 304) = 0;
          }
          *(_QWORD *)(v10 + v9 - 416) = &unk_18DCD58;
          v19 = *(_QWORD *)(v10 + v9 - 360);
          if ( v19 != 0 )
          {
            *(_QWORD *)(v10 + v9 - 360) = 0;
            --*(_QWORD *)(v19 + 16);
            v20 = (_QWORD *)(v10 + v9 - 376);
            v21 = *v20;
            *(_QWORD *)(v21 + 8) = *(_QWORD *)(v10 + v9 - 368);
            **(_QWORD **)(v10 + v9 - 368) = v21;
            v22 = (_QWORD **)(v11 - 368);
            v23 = v20;
            *v20 = v20;
            *(_QWORD *)(v10 + v9 - 368) = v20;
          }
          else
          {
            v20 = *(_QWORD **)(v10 + v9 - 376);
            v23 = *(_QWORD **)(v10 + v9 - 368);
            v22 = (_QWORD **)(v10 + v9 - 368);
          }
          v20[1] = v23;
          v9 -= 416;
          v11 -= 416;
          **v22 = v20;
        }
        while ( v9 + v25 != 0 );
      }
      sub_37BF70(v28, v4);
      v2 = v27;
      v3 = v26;
      LOBYTE(v1) = 1;
      v5 = *(_DWORD *)(v27 + 16);
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
