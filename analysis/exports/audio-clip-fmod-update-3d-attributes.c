// Updates the active channel or event instance from the engine transform.
void __fastcall audio_clip_fmod_update_3d_attributes(void *clip)
{
  __int64 v2; // rdi
  _DWORD *v3; // rax
  __int64 v4; // rdi
  _BYTE v5[12]; // [rsp+8h] [rbp-48h] BYREF
  _BYTE v6[36]; // [rsp+14h] [rbp-3Ch] BYREF
  __int64 v7; // [rsp+38h] [rbp-18h]

  v7 = 0x6365786562696C2FLL;
  if ( *((_QWORD *)clip + 50) != 0 || *((_QWORD *)clip + 53) != 0 )
  {
    v2 = *((_QWORD *)clip + 8);
    if ( v2 != 0 )
    {
      v3 = (_DWORD *)(*(__int64 (__fastcall **)(__int64))(*(_QWORD *)v2 + 136LL))(v2);
      audio_build_fmod_3d_attributes(v3, (__int64)v5);
      v4 = *((_QWORD *)clip + 50);
      if ( v4 != 0 )
        FMOD::ChannelControl::set3DAttributes(v4, v5, v6, 0);
      else
        FMOD::Studio::EventInstance::set3DAttributes(*((_QWORD *)clip + 53), v5);
    }
  }
}
