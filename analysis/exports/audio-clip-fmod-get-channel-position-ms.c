// Returns the low-level channel position in milliseconds, or zero without a channel.
float __fastcall audio_clip_fmod_get_channel_position_ms(void *clip)
{
  FMOD::Channel *v2; // rdi
  float result; // xmm0_4
  unsigned int v6; // [rsp+Ch] [rbp-14h] BYREF
  __int64 v7; // [rsp+10h] [rbp-10h]

  v7 = 0x6365786562696C2FLL;
  v6 = 0;
  v2 = *((FMOD::Channel **)clip + 50);
  if ( v2 != nullptr )
  {
    *(double *)&_XMM0 = FMOD::Channel::getPosition(v2, &v6, 1u);
    __asm { vcvtsi2ss xmm0, xmm0, rax }
  }
  else
  {
    __asm { vxorps  xmm0, xmm0, xmm0 }
  }
  LODWORD(result) = _XMM0;
  return result;
}
