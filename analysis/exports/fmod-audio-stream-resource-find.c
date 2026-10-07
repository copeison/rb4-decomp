_QWORD *__fastcall fmod_audio_stream_resource_find(_QWORD *a1, unsigned __int64 a2)
{
  _QWORD *v4; // rdx
  int v5; // eax
  _QWORD *v6; // rsi
  _QWORD *v7; // rdi
  __int64 v8; // rdi

  scePthreadMutexLock(&unk_19F2DF8);
  v4 = (_QWORD *)unk_19F2E18;
  v5 = dword_19F2DF0 + 1;
  v6 = &unk_19F2E08;
  ++dword_19F2DF0;
  if ( unk_19F2E18 != 0 )
  {
    v7 = &unk_19F2E08;
LABEL_3:
    v6 = v4;
    do
    {
      if ( v6[4] >= a2 )
      {
        v4 = (_QWORD *)v6[1];
        v7 = v6;
        if ( v4 != nullptr )
          goto LABEL_3;
        goto LABEL_8;
      }
      v6 = (_QWORD *)*v6;
    }
    while ( v6 != nullptr );
    v6 = v7;
    if ( v7 != (_QWORD *)&unk_19F2E08 )
      goto LABEL_9;
    goto LABEL_14;
  }
LABEL_8:
  if ( v6 == (_QWORD *)&unk_19F2E08 )
  {
LABEL_14:
    *a1 = 0;
    goto LABEL_15;
  }
LABEL_9:
  if ( v6[4] > a2 || v6 == (_QWORD *)&unk_19F2E08 )
    goto LABEL_14;
  v8 = v6[5];
  *a1 = v8;
  if ( v8 != 0 )
  {
    sub_1ADEB0(v8);
    v5 = dword_19F2DF0;
  }
LABEL_15:
  dword_19F2DF0 = v5 - 1;
  scePthreadMutexUnlock(&unk_19F2DF8);
  return a1;
}
