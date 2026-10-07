// Stores the integer sample rate used by output-block callbacks.
void __fastcall audio_output_dispatcher_set_sample_rate(void *dispatcher, int sample_rate)
{
  *((_DWORD *)dispatcher + 28) = sample_rate;
}
