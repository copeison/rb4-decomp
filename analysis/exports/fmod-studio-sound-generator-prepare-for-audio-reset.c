__int64 __fastcall fmod_studio_sound_generator_prepare_for_audio_reset(__int64 a1)
{
  __int64 v2; // rdi
  __int64 result; // rax

  v2 = *(_QWORD *)(a1 + 192);
  if ( v2 != 0 )
  {
    result = FMOD::Studio::EventInstance::stop(v2, *(_DWORD *)(a1 + 28) == 4);
    *(_DWORD *)(a1 + 28) = 6;
  }
  return result;
}
