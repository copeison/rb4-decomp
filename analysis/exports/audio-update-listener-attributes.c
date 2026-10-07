// Convert the engine listener transform and apply it to FMOD Studio listener 0.
__int64 __fastcall audio_update_listener_attributes(__int64 a1, _DWORD *a2)
{
  _BYTE v3[48]; // [rsp+0h] [rbp-50h] BYREF
  __int64 v4; // [rsp+30h] [rbp-20h]

  v4 = 0x6365786562696C2FLL;
  if ( *(_BYTE *)(a1 + 14) != 0 && unk_19F29D8 != 0 && *(_QWORD *)(unk_19F29D8 + 280LL) != 0 )
  {
    audio_build_fmod_3d_attributes(a2, (__int64)v3);
    FMOD::Studio::System::setListenerAttributes(*(_QWORD *)(unk_19F29D8 + 280LL), 0, v3);
  }
  return 0x6365786562696C2FLL;
}
