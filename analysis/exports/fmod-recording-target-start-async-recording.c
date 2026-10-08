// Starts the generic recording worker with the async_audio_record settings and RecordingAudioRenderTarget thread name.
__int64 __fastcall fmod_recording_target_start_async_recording(__int64 a1)
{
  _QWORD v3[4]; // [rsp+0h] [rbp-20h] BYREF

  v3[1] = 0x6365786562696C2FLL;
  thread_affinity_find_group("async_audio_record", v3);
  sub_259210(
    a1 + 1312,
    (unsigned int)fmod_recording_target_thread_entry,
    a1,
    (unsigned int)"RecordingAudioRenderTarget",
    *(_QWORD *)(v3[0] + 24LL),
    *(_DWORD *)(v3[0] + 32LL),
    *(_DWORD *)(v3[0] + 16LL),
    *(_QWORD *)(v3[0] + 40LL));
  sub_25C430(a1 + 1320);
  return 0x6365786562696C2FLL;
}
