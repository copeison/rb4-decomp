__int64 __fastcall audio_clip_fmod_dsp_set_position(_QWORD *a1)
{
  FMOD::DSP::setChannelFormat(*a1, 3, 2, 3);
  return 0;
}
