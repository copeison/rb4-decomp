// Runs both low-level-channel and Studio-event release paths.
void __fastcall audio_clip_fmod_process_release(void *clip)
{
  audio_clip_fmod_defer_channel_release(clip);
  audio_clip_fmod_release_event_instance(clip);
}
