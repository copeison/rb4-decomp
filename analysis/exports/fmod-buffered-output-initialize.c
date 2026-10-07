int __fastcall fmod_buffered_output_initialize(
        void *output_state,
        int selected_driver,
        unsigned int flags,
        int *output_rate,
        int *speaker_mode,
        int *speaker_mode_channels,
        int *output_format,
        int dsp_buffer_length,
        int dsp_buffer_count,
        void *extra_driver_data)
{
  *(_QWORD *)output_state = extra_driver_data;
  *output_format = 5;
  *speaker_mode = *((_DWORD *)extra_driver_data + 75);
  *speaker_mode_channels = *((_DWORD *)extra_driver_data + 54);
  return 0;
}
