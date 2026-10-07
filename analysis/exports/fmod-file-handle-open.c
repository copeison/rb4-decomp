__int64 __fastcall fmod_file_handle_open(__int64 a1, _BYTE *a2, _DWORD *a3)
{
  __int64 v6; // r14
  _BYTE *v7; // rbx
  unsigned __int64 v8; // rbx
  __int64 v9; // rdi
  _QWORD *v10; // rax
  unsigned int v11; // ebx

  v6 = a1 + 24;
  scePthreadMutexLock(a1 + 24);
  ++*(_DWORD *)(a1 + 16);
  v7 = *(_BYTE **)(a1 + 8);
  if ( v7 != a2 )
  {
    if ( a2 != nullptr && *a2 != 0 )
    {
      v8 = strlen(a2);
      (*(void (__fastcall **)(__int64, unsigned __int64))(*(_QWORD *)a1 + 24LL))(a1, v8);
      v9 = *(_QWORD *)(a1 + 8);
      if ( *(unsigned int *)(v9 - 4) < v8 )
        v8 = *(unsigned int *)(v9 - 4);
      memmove(v9, a2, v8);
      v7 = (_BYTE *)(*(_QWORD *)(a1 + 8) + v8);
    }
    *v7 = 0;
  }
  v10 = (_QWORD *)engine_file_open((__int64)a2, 2);
  *(_QWORD *)(a1 + 32) = v10;
  if ( v10 != nullptr )
  {
    v10[1] = 0;
    (*(void (__fastcall **)(_QWORD *))(*v10 + 136LL))(v10);
    v11 = 0;
    *a3 = engine_file_get_size(*(_QWORD *)(a1 + 32));
  }
  else
  {
    v11 = 18;
  }
  --*(_DWORD *)(a1 + 16);
  scePthreadMutexUnlock(v6);
  return v11;
}
