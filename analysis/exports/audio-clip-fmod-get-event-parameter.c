// Gets a named Studio event parameter through its ParameterInstance.
bool __fastcall audio_clip_fmod_get_event_parameter(void *clip, const char *name, float *value)
{
  __int64 v4; // rdi
  bool result; // al
  FMOD::Studio::ParameterInstance *v6[4]; // [rsp+0h] [rbp-20h] BYREF

  v6[1] = (FMOD::Studio::ParameterInstance *)0x6365786562696C2FLL;
  v4 = *((_QWORD *)clip + 53);
  result = v4 != 0
        && (v6[0] = nullptr, (unsigned int)FMOD::Studio::EventInstance::getParameter(v4, name, v6) == 0)
        && (unsigned int)FMOD::Studio::ParameterInstance::getValue(v6[0], value) == 0;
  return result;
}
