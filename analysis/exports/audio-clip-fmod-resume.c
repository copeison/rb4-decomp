// Resumes the active low-level channel or Studio event.
void __fastcall audio_clip_fmod_resume(void *clip)
{
  FMOD::ChannelControl *v2; // rdi
  FMOD::Studio::EventInstance *v3; // rdi
  int v4; // eax

  if ( (unsigned int)(*((_DWORD *)clip + 7) - 5) >= 2 )
  {
    v2 = *((FMOD::ChannelControl **)clip + 50);
    if ( v2 != nullptr )
    {
      FMOD::ChannelControl::setPaused(v2, false);
    }
    else
    {
      v3 = *((FMOD::Studio::EventInstance **)clip + 53);
      if ( v3 == nullptr )
      {
        v4 = 2;
        goto LABEL_7;
      }
      FMOD::Studio::EventInstance::setPaused(v3, false);
    }
    v4 = 3;
LABEL_7:
    *((_DWORD *)clip + 7) = v4;
  }
}
