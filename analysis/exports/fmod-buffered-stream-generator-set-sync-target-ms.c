int __fastcall fmod_buffered_stream_generator_set_sync_target_ms(__int64 a1, double a2, __m128 _XMM1)
{
  int result; // eax

  __asm
  {
    vxorps  xmm1, xmm1, xmm1
    vmaxss  xmm0, xmm1, xmm0
    vmulss  xmm0, xmm0, cs:dword_125CDF4
    vmulss  xmm0, xmm0, dword ptr [rdi+1D4h]
    vcvttss2si eax, xmm0
  }
  *(_DWORD *)(a1 + 528) = result;
  return result;
}
