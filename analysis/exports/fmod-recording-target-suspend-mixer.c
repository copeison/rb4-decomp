__int64 __fastcall fmod_recording_target_suspend_mixer(__int64 a1)
{
  return FMOD::System::mixerSuspend(*(FMOD::System **)(a1 + 768));
}
