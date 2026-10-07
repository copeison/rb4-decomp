bool __fastcall fmod_studio_sound_generator_get_event_parameter(__int64 a1, __int64 a2, float *a3)
{
  __int64 v5; // rdi
  FMOD::Studio::ParameterInstance *v7[4]; // [rsp+0h] [rbp-20h] BYREF

  v7[1] = (FMOD::Studio::ParameterInstance *)0x6365786562696C2FLL;
  v5 = *(_QWORD *)(a1 + 192);
  if ( v5 == 0 || *(_DWORD *)(a1 + 28) == 6 )
    return false;
  v7[0] = nullptr;
  return (unsigned int)FMOD::Studio::EventInstance::getParameter(v5, a2, v7) == 0
      && (unsigned int)FMOD::Studio::ParameterInstance::getValue(v7[0], a3) == 0;
}
