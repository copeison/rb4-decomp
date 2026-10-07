// Maps four engine speaker configurations to FMOD mode and raw channel count.
void __fastcall fmod_audio_configure_speakers(void *state, int configuration)
{
  int v2; // eax

  if ( (unsigned int)configuration <= 3 )
  {
    v2 = *((_DWORD *)qword_125D200 + configuration);
    *((_DWORD *)state + 75) = *((_DWORD *)qword_125D1F0 + configuration);
    *((_DWORD *)state + 54) = v2;
  }
}
