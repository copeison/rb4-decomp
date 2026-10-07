__int64 __fastcall fmod_audio_stream_resource_unregister(__int64 a1)
{
  __int64 v2; // r13
  __int64 v3; // rbx
  __int64 v4; // r15
  char v6; // [rsp+14h] [rbp-2Ch]

  scePthreadMutexLock(&unk_19F2DF8);
  ++dword_19F2DF0;
  v6 = 0;
  v2 = unk_19F2E10;
  while ( 1 )
  {
    v3 = v2;
    if ( (_UNKNOWN *)v2 == &unk_19F2E08 )
      break;
    v4 = *(_QWORD *)(v2 + 40);
    v2 = sub_253100(v2);
    if ( v4 == a1 )
    {
      --unk_19F2E28;
      sub_253100(v3);
      sub_2534A0(v3, &unk_19F2E08);
      sub_252D30(&unk_19F2E30, v3, 48);
      v6 = 1;
    }
  }
  LOBYTE(v3) = v6 != 0;
  --dword_19F2DF0;
  scePthreadMutexUnlock(&unk_19F2DF8);
  return (unsigned int)v3;
}
