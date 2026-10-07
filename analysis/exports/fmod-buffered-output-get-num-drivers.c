int __fastcall fmod_buffered_output_get_num_drivers(void *output_state, int *driver_count)
{
  *driver_count = 128;
  return 0;
}
