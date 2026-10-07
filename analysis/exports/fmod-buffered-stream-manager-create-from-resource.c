__int64 *__fastcall fmod_buffered_stream_manager_create_from_resource(__int64 a1, __int64 a2)
{
  __int64 *v4; // rbx
  __int64 v5; // r15
  __int64 v7; // [rsp+8h] [rbp-38h] BYREF
  _QWORD v8[6]; // [rsp+10h] [rbp-30h] BYREF

  v8[1] = 0x6365786562696C2FLL;
  fmod_audio_stream_resource_find(v8, *(_QWORD *)a2);
  if ( v8[0] == 0 )
    return nullptr;
  if ( (*(unsigned __int8 (__fastcall **)(_QWORD))(*(_QWORD *)v8[0] + 48LL))(v8[0]) != 0 )
  {
    v4 = nullptr;
  }
  else if ( *(_DWORD *)(a2 + 96) == 3 && *(_BYTE *)(a2 + 101) != 0 )
  {
    v5 = v8[0];
    v7 = v8[0];
    if ( v8[0] != 0 )
    {
      sub_1ADEB0(v8[0]);
      v4 = fmod_buffered_stream_manager_create_from_options(a1, &v7, a2);
      sub_1ADEF0(v5);
    }
    else
    {
      v4 = fmod_buffered_stream_manager_create_from_options(a1, &v7, a2);
    }
  }
  else
  {
    v4 = nullptr;
  }
  if ( v8[0] != 0 )
    sub_1ADEF0(v8[0]);
  return v4;
}
