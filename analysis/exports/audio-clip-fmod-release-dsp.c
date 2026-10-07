// Clears routing handles and immediately releases the custom DSP.
void __fastcall audio_clip_fmod_release_dsp(void *clip)
{
  FMOD::DSP *v4; // rdi

  _RBX = clip;
  __asm
  {
    vxorps  xmm0, xmm0, xmm0
    vmovups xmmword ptr [rbx+198h], xmm0
  }
  v4 = *((FMOD::DSP **)clip + 49);
  if ( v4 != nullptr )
    FMOD::DSP::release(v4);
  _RBX[49] = 0;
}
