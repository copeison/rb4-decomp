// Advances deferred teardown or updates the active voice's 3D attributes.
bool __fastcall audio_clip_fmod_update(void *clip)
{
  int v2; // eax
  bool v3; // r14
  __int64 v4; // rdi
  _DWORD *v5; // rax
  __int64 v6; // rdi
  _BYTE v8[12]; // [rsp+0h] [rbp-50h] BYREF
  _BYTE v9[36]; // [rsp+Ch] [rbp-44h] BYREF
  __int64 v10; // [rsp+30h] [rbp-20h]

  v10 = 0x6365786562696C2FLL;
  v2 = *((_DWORD *)clip + 7);
  if ( v2 == 5 )
    return false;
  v3 = true;
  if ( *((_BYTE *)clip + 444) == 0 )
  {
    if ( v2 == 6 )
    {
      audio_clip_fmod_defer_channel_release(clip);
      audio_clip_fmod_release_event_instance(clip);
      return *((_DWORD *)clip + 7) != 5;
    }
    else if ( *((_QWORD *)clip + 50) != 0 || *((_QWORD *)clip + 53) != 0 )
    {
      v4 = *((_QWORD *)clip + 8);
      if ( v4 != 0 )
      {
        v5 = (_DWORD *)(*(__int64 (__fastcall **)(__int64))(*(_QWORD *)v4 + 136LL))(v4);
        audio_build_fmod_3d_attributes(v5, (__int64)v8);
        v6 = *((_QWORD *)clip + 50);
        if ( v6 != 0 )
          FMOD::ChannelControl::set3DAttributes(v6, v8, v9, 0);
        else
          FMOD::Studio::EventInstance::set3DAttributes(*((_QWORD *)clip + 53), v8);
      }
    }
  }
  return v3;
}
