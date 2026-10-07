// Sets low-level channel position or Studio event timeline position in milliseconds.
// local variable allocation has failed, the output may be wrong!
void __fastcall audio_clip_fmod_set_position_ms(void *clip, float milliseconds)
{
  FMOD::Channel *v3; // rdi
  FMOD::Studio::EventInstance *v5; // rdi

  v3 = *((FMOD::Channel **)clip + 50);
  if ( v3 != nullptr )
  {
    __asm { vcvttss2si rsi, xmm0 }
    FMOD::Channel::setPosition(v3, _RSI, 1u);
  }
  else
  {
    v5 = *((FMOD::Studio::EventInstance **)clip + 53);
    if ( v5 != nullptr )
    {
      __asm { vcvttss2si esi, xmm0 }
      FMOD::Studio::EventInstance::setTimelinePosition(v5, _ESI);
    }
  }
}
