// local variable allocation has failed, the output may be wrong!
int __fastcall fmod_buffered_output_get_driver_info(
        void *output_state,
        int id,
        char *name,
        int name_length,
        void *guid,
        int *system_rate,
        int *speaker_mode,
        int *speaker_mode_channels)
{
  __int64 v10; // r8
  __int64 v11; // r15

  v10 = (unsigned int)id;
  if ( name != nullptr )
  {
    v11 = name_length;
    sub_9290((_DWORD)name, name_length, (unsigned int)"null_output_%d", id, id, (_DWORD)system_rate);
    name[v11 - 1] = 0;
  }
  *(double *)&_XMM0 = audio_get_sample_rate(output_state, *(_QWORD *)&id, name, *(_QWORD *)&name_length, v10);
  __asm { vcvttsd2si eax, xmm0 }
  *system_rate = _EAX;
  *speaker_mode = 3;
  *speaker_mode_channels = 2;
  return 0;
}
