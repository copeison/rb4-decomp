// Pauses the active low-level channel or Studio event and enters the paused state.
void __fastcall audio_clip_fmod_pause(void *clip)
{
  FMOD::ChannelControl *v2; // rdi
  FMOD::Studio::EventInstance *v3; // rdi

  if ( (unsigned int)(*((_DWORD *)clip + 7) - 4) >= 3 )
  {
    v2 = *((FMOD::ChannelControl **)clip + 50);
    if ( v2 != nullptr )
    {
      FMOD::ChannelControl::setPaused(v2, true);
    }
    else
    {
      v3 = *((FMOD::Studio::EventInstance **)clip + 53);
      if ( v3 != nullptr )
        FMOD::Studio::EventInstance::setPaused(v3, true);
    }
    *((_DWORD *)clip + 7) = 4;
  }
}
