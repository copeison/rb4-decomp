bool __fastcall fmod_studio_sound_generator_set_event_parameter(__int64 a1, __int64 a2, double a3)
{
  __int64 v4; // rdi

  v4 = *(_QWORD *)(a1 + 192);
  if ( v4 == 0 )
    return false;
  if ( *(_DWORD *)(a1 + 28) == 6 )
    return false;
  return (unsigned int)FMOD::Studio::EventInstance::setParameterValue(v4, a2, a3) == 0;
}
