__int64 __fastcall fmod_audio_stream_generator_update_3d(__int64 a1)
{
  __int64 v2; // rdi
  _DWORD *v3; // rax
  _BYTE v5[12]; // [rsp+0h] [rbp-50h] BYREF
  _BYTE v6[36]; // [rsp+Ch] [rbp-44h] BYREF
  __int64 v7; // [rsp+30h] [rbp-20h]

  v7 = 0x6365786562696C2FLL;
  if ( *(_QWORD *)(a1 + 88) != 0 )
  {
    v2 = *(_QWORD *)(a1 + 64);
    if ( v2 != 0 )
    {
      v3 = (_DWORD *)(*(__int64 (__fastcall **)(__int64))(*(_QWORD *)v2 + 136LL))(v2);
      audio_build_fmod_3d_attributes(v3, (__int64)v5);
      FMOD::ChannelControl::set3DAttributes(*(_QWORD *)(a1 + 88), v5, v6, 0);
    }
  }
  return 0x6365786562696C2FLL;
}
