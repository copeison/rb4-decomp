void *__fastcall fmod_audio_input_manager_destruct(_QWORD *a1)
{
  void *result; // rax
  __int64 v2; // rsi

  result = &vtable_FmodAudioInputManager;
  *a1 = &vtable_FmodAudioInputManager;
  v2 = a1[2];
  if ( v2 != 0 )
    return (void *)sub_252D30(a1 + 5, v2, a1[4] - v2);
  return result;
}
