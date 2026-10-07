void __fastcall fmod_audio_stream_generator_set_loop_points(__int64 _RDI, __m128 _XMM0, __m128 _XMM1)
{
  __asm
  {
    vmovss  dword ptr [rdi+104h], xmm0
    vmovss  dword ptr [rdi+108h], xmm1
  }
  *(_BYTE *)(_RDI + 268) = 1;
}
