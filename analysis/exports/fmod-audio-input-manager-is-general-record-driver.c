_BOOL4 __fastcall fmod_audio_input_manager_is_general_record_driver(__int64 a1, __int64 a2)
{
  unsigned __int64 v3; // rax

  v3 = strlen(a2);
  return v3 >= 7 && (unsigned int)strncmp(a2 + v3 - 7, "GENERAL", 7) == 0;
}
