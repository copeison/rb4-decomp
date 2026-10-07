__int64 __fastcall fmod_audio_stream_generator_delete(_QWORD *a1)
{
  __int64 v2; // rcx
  _QWORD *v3; // rax
  _QWORD **v4; // rcx
  __int64 v5; // rdx
  _QWORD *v6; // rdx

  *a1 = &unk_18F0418;
  sub_406D0((__int64)a1);
  *a1 = &unk_18DCD58;
  v2 = a1[7];
  if ( v2 != 0 )
  {
    a1[7] = 0;
    --*(_QWORD *)(v2 + 16);
    v3 = a1 + 5;
    v4 = (_QWORD **)(a1 + 6);
    v5 = a1[5];
    *(_QWORD *)(v5 + 8) = a1[6];
    *(_QWORD *)a1[6] = v5;
    v6 = a1 + 5;
    a1[5] = a1 + 5;
    a1[6] = a1 + 5;
  }
  else
  {
    v3 = (_QWORD *)a1[5];
    v6 = (_QWORD *)a1[6];
    v4 = (_QWORD **)(a1 + 6);
  }
  v3[1] = v6;
  **v4 = v3;
  return sub_37BF50(a1);
}
