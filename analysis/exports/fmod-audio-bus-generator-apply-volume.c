__int64 __fastcall fmod_audio_bus_generator_apply_volume(double (__fastcall ***a1)(FMOD::ChannelControl **))
{
  *(double *)&_XMM0 = (*a1)[13]((FMOD::ChannelControl **)a1);
  __asm { vmulss  xmm0, xmm0, dword ptr [rbx+0CCh] }
  return FMOD::ChannelControl::setVolume((FMOD::ChannelControl *)a1[11], *(float *)&_XMM0);
}
