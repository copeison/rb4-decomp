__int64 __fastcall fmod_file_async_cancel(__int64 a1)
{
  __int64 v2; // rdx
  _QWORD *v3; // rdi
  unsigned __int64 v4; // rcx

  if ( a1 == 0 )
    return 31;
  scePthreadMutexLock(&unk_19F3020);
  v2 = *((_QWORD *)&xmmword_19F3028 + 1);
  v3 = (_QWORD *)xmmword_19F3028;
  ++dword_19F3018;
  if ( *((_QWORD *)&xmmword_19F3028 + 1) == (_QWORD)xmmword_19F3028 )
  {
LABEL_6:
    while ( qword_19F3050 == a1 )
      scePthreadCondWait(&unk_19F3010, qword_19F3008);
  }
  else
  {
    v4 = 0;
    while ( *v3 != a1 )
    {
      ++v4;
      ++v3;
      if ( v4 >= (__int64)(*((_QWORD *)&xmmword_19F3028 + 1) - xmmword_19F3028) >> 3 )
        goto LABEL_6;
    }
    if ( (unsigned __int64)(v3 + 1) < *((_QWORD *)&xmmword_19F3028 + 1) )
    {
      memmove(v3, v3 + 1, *((_QWORD *)&xmmword_19F3028 + 1) - (_QWORD)(v3 + 1));
      v2 = *((_QWORD *)&xmmword_19F3028 + 1);
    }
    *((_QWORD *)&xmmword_19F3028 + 1) = v2 - 8;
  }
  --dword_19F3018;
  scePthreadMutexUnlock(&unk_19F3020);
  return 0;
}
