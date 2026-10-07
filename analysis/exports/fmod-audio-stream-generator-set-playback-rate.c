void __fastcall fmod_audio_stream_generator_set_playback_rate(__int64 _RDI, __m128 _XMM0)
{
  char v2; // zf

  __asm { vucomiss xmm0, dword ptr [rdi+118h] }
  if ( !v2 )
  {
    __asm { vmovss  dword ptr [rdi+118h], xmm0 }
    *(_BYTE *)(_RDI + 284) = 1;
  }
}
