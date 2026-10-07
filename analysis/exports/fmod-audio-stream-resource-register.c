__int64 __fastcall fmod_audio_stream_resource_register(__int64 a1)
{
  __int64 v2; // rax
  double v3; // xmm0_8
  _QWORD *v4; // rdi
  _QWORD *v5; // rsi
  _QWORD *v6; // rcx
  void *v7; // rdx
  __int64 v9; // [rsp+8h] [rbp-28h] BYREF
  _QWORD v10[4]; // [rsp+10h] [rbp-20h] BYREF

  v10[1] = 0x6365786562696C2FLL;
  scePthreadMutexLock(&unk_19F2DF8);
  ++dword_19F2DF0;
  v2 = sub_2453D0(*(_QWORD *)(a1 + 48));
  v3 = sub_256FD0(v10, v2);
  v4 = (_QWORD *)unk_19F2E18;
  v9 = v10[0];
  if ( unk_19F2E18 == 0 )
  {
    v7 = &unk_19F2E08;
LABEL_10:
    sub_273180(v10, &unk_19F2E00, v7, &v9, v3);
    v6 = (_QWORD *)v10[0];
    goto LABEL_11;
  }
  v5 = &unk_19F2E08;
  do
  {
    v6 = v4;
    while ( v6[4] < v10[0] )
    {
      v6 = (_QWORD *)*v6;
      if ( v6 == nullptr )
      {
        v6 = v5;
        goto LABEL_8;
      }
    }
    v4 = (_QWORD *)v6[1];
    v5 = v6;
  }
  while ( v4 != nullptr );
LABEL_8:
  v7 = &unk_19F2E08;
  if ( v6 == (_QWORD *)&unk_19F2E08 )
    goto LABEL_10;
  v7 = v6;
  if ( v10[0] < v6[4] )
    goto LABEL_10;
LABEL_11:
  v6[5] = a1;
  --dword_19F2DF0;
  scePthreadMutexUnlock(&unk_19F2DF8);
  return 0x6365786562696C2FLL;
}
