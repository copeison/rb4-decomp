__int64 fmod_async_file_reader_thread()
{
  int v0; // eax
  __int64 v1; // rax
  double v2; // xmm0_8
  __int64 v3; // r14
  int v4; // r13d
  __int64 v5; // rbx
  __int64 v6; // r15
  unsigned int v7; // r12d
  unsigned int v8; // eax
  char *v9; // rsi

  scePthreadMutexLock(&unk_19F3020);
  v0 = ++dword_19F3018;
  if ( byte_19F3048 == 0 )
  {
    while ( 1 )
    {
      if ( (_QWORD)xmmword_19F3028 == *((_QWORD *)&xmmword_19F3028 + 1) )
      {
        if ( qword_19F3050 != 0 )
          goto LABEL_6;
LABEL_15:
        scePthreadCondWait(&unk_19F3010, qword_19F3008);
        if ( byte_19F3048 == 1 )
          goto LABEL_16;
      }
      else
      {
        v1 = *(_QWORD *)(*((_QWORD *)&xmmword_19F3028 + 1) - 8LL);
        *((_QWORD *)&xmmword_19F3028 + 1) -= 8LL;
        qword_19F3050 = v1;
        if ( v1 == 0 )
          goto LABEL_15;
LABEL_6:
        --dword_19F3018;
        v2 = scePthreadMutexUnlock(&unk_19F3020);
        v3 = qword_19F3050;
        v4 = 31;
        v5 = *(_QWORD *)qword_19F3050;
        if ( *(_QWORD *)qword_19F3050 != 0 )
        {
          scePthreadMutexLock(v5 + 24);
          ++*(_DWORD *)(v5 + 16);
          engine_file_seek(*(_QWORD *)(v5 + 32));
          --*(_DWORD *)(v5 + 16);
          v2 = scePthreadMutexUnlock(v5 + 24);
          v3 = qword_19F3050;
          v6 = *(_QWORD *)qword_19F3050;
          if ( *(_QWORD *)qword_19F3050 != 0 )
          {
            v7 = *(_DWORD *)(qword_19F3050 + 12);
            scePthreadMutexLock(v6 + 24);
            ++*(_DWORD *)(v6 + 16);
            v8 = engine_file_read(*(_QWORD *)(v6 + 32));
            *(_DWORD *)(v3 + 40) = v8;
            --*(_DWORD *)(v6 + 16);
            v4 = v8 < v7 ? 0x10 : 0;
            v2 = scePthreadMutexUnlock(v6 + 24);
            v3 = qword_19F3050;
          }
        }
        v9 = (_BYTE *)(&xmmword_0 + 13);
        if ( v4 == 0 )
          v9 = nullptr;
        if ( v4 == 16 )
          v9 = "so.1";
        (*(void (__fastcall **)(__int64, char *, double))(v3 + 48))(v3, v9, v2);
        scePthreadMutexLock(&unk_19F3020);
        ++dword_19F3018;
        qword_19F3050 = 0;
        scePthreadCondSignal(&unk_19F3010);
        if ( byte_19F3048 == 1 )
        {
LABEL_16:
          v0 = dword_19F3018;
          break;
        }
      }
    }
  }
  dword_19F3018 = v0 - 1;
  scePthreadMutexUnlock(&unk_19F3020);
  return 0;
}
