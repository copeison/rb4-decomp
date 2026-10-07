__int64 fmod_audio_bus_manager_extension_symbol()
{
  __int64 result; // rax
  _QWORD v1[4]; // [rsp+0h] [rbp-20h] BYREF

  v1[1] = 0x6365786562696C2FLL;
  if ( byte_19F2CC0[0] == 0 && (unsigned int)_cxa_guard_acquire(byte_19F2CC0) != 0 )
  {
    unk_19F2CB8 = 19246190;
    _cxa_guard_release(byte_19F2CC0);
  }
  result = unk_19F2CB8;
  if ( unk_19F2CB8 == 19246190 )
  {
    sub_256FD0(v1, ".fmod_bus");
    unk_19F2CB8 = v1[0];
    return v1[0];
  }
  return result;
}
