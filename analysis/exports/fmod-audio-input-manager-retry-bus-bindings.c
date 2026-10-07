unsigned __int64 __fastcall fmod_audio_input_manager_retry_bus_bindings(__int64 a1)
{
  unsigned __int64 result; // rax

  if ( *(_BYTE *)(a1 + 48) != 0 )
    return fmod_audio_input_manager_bind_buses(a1);
  return result;
}
