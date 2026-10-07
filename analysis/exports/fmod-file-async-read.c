__int64 __fastcall fmod_file_async_read(__int64 a1)
{
  __int64 v2; // rsi
  _QWORD v4[4]; // [rsp+0h] [rbp-20h] BYREF

  v4[1] = 0x6365786562696C2FLL;
  v4[0] = a1;
  if ( a1 == 0 )
    return 31;
  scePthreadMutexLock(&unk_19F3020);
  v2 = xmmword_19F3028;
  ++dword_19F3018;
  if ( (_QWORD)xmmword_19F3028 != *((_QWORD *)&xmmword_19F3028 + 1) )
  {
    while ( *(_DWORD *)(*(_QWORD *)v2 + 16LL) < *(_DWORD *)(a1 + 16) )
    {
      v2 += 8;
      if ( *((_QWORD *)&xmmword_19F3028 + 1) == v2 )
      {
        v2 = *((_QWORD *)&xmmword_19F3028 + 1);
        break;
      }
    }
  }
  if ( *((_QWORD *)&xmmword_19F3028 + 1) != v2 || *((_QWORD *)&xmmword_19F3028 + 1) == qword_19F3038 )
  {
    sub_27A920(&xmmword_19F3028, v2, v4);
  }
  else
  {
    **((_QWORD **)&xmmword_19F3028 + 1) = a1;
    *((_QWORD *)&xmmword_19F3028 + 1) += 8LL;
  }
  scePthreadCondSignal(&unk_19F3010);
  --dword_19F3018;
  scePthreadMutexUnlock(&unk_19F3020);
  return 0;
}
