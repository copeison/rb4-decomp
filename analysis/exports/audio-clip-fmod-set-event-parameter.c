// Sets a named Studio event parameter when an event instance exists.
// local variable allocation has failed, the output may be wrong!
bool __fastcall audio_clip_fmod_set_event_parameter(void *clip, const char *name, float value)
{
  __int64 v3; // rdi

  v3 = *((_QWORD *)clip + 53);
  return v3 != 0 && (unsigned int)FMOD::Studio::EventInstance::setParameterValue(v3, name, *(double *)&value) == 0;
}
