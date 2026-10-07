unsigned __int64 __fastcall fmod_audio_input_manager_bind_buses(__int64 a1)
{
  __int64 v2; // rdx
  unsigned __int64 result; // rax
  signed __int64 v4; // rbx
  __int64 v5; // r15
  __int64 v6; // r14
  char v7; // r13
  __int64 v8; // r14
  __int64 v9; // rbx

  v2 = *(_QWORD *)(a1 + 16);
  result = *(_QWORD *)(a1 + 24) - v2;
  if ( result != 0 && (int)(result = -1431655765 * (unsigned int)(result >> 3)) > 0 )
  {
    v4 = 0;
    v5 = 8;
    v6 = *(_QWORD *)(unk_19F29D8 + 280LL);
    while ( 1 )
    {
      result = FMOD::Studio::System::getBus(v6, *(_QWORD *)(v2 + v5 - 8), v5 + v2);
      if ( (_DWORD)result != 0 )
        break;
      result = (*(__int64 (__fastcall **)(_QWORD *, _QWORD))(*qword_19C9088 + 16LL))(
                 qword_19C9088,
                 *(_QWORD *)(*(_QWORD *)(a1 + 16) + v5 - 8));
      if ( (_BYTE)result == 0 )
        break;
      v7 = 0;
      FMOD::Studio::Bus::getVolume(
        *(FMOD::Studio::Bus **)(*(_QWORD *)(a1 + 16) + v5),
        (float *)(*(_QWORD *)(a1 + 16) + v5 + 8),
        nullptr);
      v2 = *(_QWORD *)(a1 + 16);
      ++v4;
      v5 += 24;
      result = (int)(-1431655765 * ((unsigned __int64)(*(_QWORD *)(a1 + 24) - v2) >> 3));
      if ( v4 >= (__int64)result )
        goto LABEL_9;
    }
    v7 = 1;
    if ( (int)v4 > 0 )
    {
      v8 = (unsigned int)v4;
      v9 = 0;
      do
      {
        result = (*(__int64 (__fastcall **)(_QWORD *, _QWORD))(*qword_19C9088 + 24LL))(
                   qword_19C9088,
                   *(_QWORD *)(*(_QWORD *)(a1 + 16) + v9));
        v9 += 24;
        --v8;
      }
      while ( v8 != 0 );
    }
  }
  else
  {
    v7 = 0;
  }
LABEL_9:
  *(_BYTE *)(a1 + 48) = v7;
  return result;
}
