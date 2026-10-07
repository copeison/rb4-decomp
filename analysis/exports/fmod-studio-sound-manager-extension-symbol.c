__int64 fmod_studio_sound_manager_extension_symbol()
{
  __int64 result; // rax
  _QWORD v1[4]; // [rsp+0h] [rbp-20h] BYREF

  v1[1] = 0x6365786562696C2FLL;
  if ( byte_19F2D78[16] == 0 && (unsigned int)_cxa_guard_acquire(&byte_19F2D78[16]) != 0 )
  {
    *(_QWORD *)&byte_19F2D78[8] = 19246190;
    _cxa_guard_release(&byte_19F2D78[16]);
  }
  result = *(_QWORD *)&byte_19F2D78[8];
  if ( *(_QWORD *)&byte_19F2D78[8] == 19246190 )
  {
    sub_256FD0(v1, ".bank");
    *(_QWORD *)&byte_19F2D78[8] = v1[0];
    return v1[0];
  }
  return result;
}
