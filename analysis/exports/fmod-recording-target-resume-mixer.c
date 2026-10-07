__int64 __fastcall fmod_recording_target_resume_mixer(__int64 a1)
{
  return FMOD::System::mixerResume(*(FMOD::System **)(a1 + 768));
}
