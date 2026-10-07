_QWORD *__fastcall fmod_audio_stream_generator_destruct(_QWORD *a1)
{
  __int64 v2; // rcx
  _QWORD *result; // rax
  __int64 v4; // rcx
  _QWORD **v5; // rcx
  _QWORD *v6; // rdx

  *a1 = &unk_18F0418;
  sub_406D0((__int64)a1);
  *a1 = &unk_18DCD58;
  v2 = a1[7];
  if ( v2 != 0 )
  {
    a1[7] = 0;
    --*(_QWORD *)(v2 + 16);
    result = a1 + 5;
    v4 = a1[5];
    *(_QWORD *)(v4 + 8) = a1[6];
    *(_QWORD *)a1[6] = v4;
    v5 = (_QWORD **)(a1 + 6);
    v6 = a1 + 5;
    a1[5] = a1 + 5;
    a1[6] = a1 + 5;
  }
  else
  {
    result = (_QWORD *)a1[5];
    v6 = (_QWORD *)a1[6];
    v5 = (_QWORD **)(a1 + 6);
  }
  result[1] = v6;
  **v5 = result;
  return result;
}
