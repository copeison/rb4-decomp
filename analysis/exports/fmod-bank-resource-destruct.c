__int64 __fastcall fmod_bank_resource_destruct(_QWORD *a1)
{
  __int128 *v2; // rbx
  int v3; // eax
  bool v4; // zf
  __int64 *v5; // rsi
  __int64 v6; // rax
  double v7; // xmm0_8

  *a1 = &vtable_FModBankResource;
  scePthreadMutexLock(&unk_19F2E48);
  v2 = (__int128 *)xmmword_19F2E50;
  v3 = ++dword_19F2E40;
  if ( (__int128 *)xmmword_19F2E50 != &xmmword_19F2E50 )
  {
    do
    {
      while ( 1 )
      {
        v4 = *((_QWORD *)v2 + 2) == (_QWORD)a1;
        v2 = *(__int128 **)v2;
        if ( v4 )
          break;
        if ( v2 == &xmmword_19F2E50 )
          goto LABEL_6;
      }
      v5 = *((__int64 **)v2 + 1);
      v6 = *v5;
      *(_QWORD *)(v6 + 8) = v5[1];
      *(_QWORD *)v5[1] = v6;
      sub_252D30(&unk_19F2E68, v5, 24);
      --qword_19F2E60;
    }
    while ( v2 != &xmmword_19F2E50 );
LABEL_6:
    v3 = dword_19F2E40;
  }
  dword_19F2E40 = v3 - 1;
  v7 = scePthreadMutexUnlock(&unk_19F2E48);
  fmod_bank_resource_unload_all(a1, v7);
  fmod_bank_resource_clear_bank_map((__int64)(a1 + 6), (_QWORD *)a1[9]);
  return sub_1ADD70(a1);
}
