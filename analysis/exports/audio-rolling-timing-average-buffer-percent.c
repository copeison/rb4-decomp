// Returns mean rolling-window time normalized by window size and audio-buffer duration.
double __fastcall audio_rolling_timing_average_buffer_percent(const void *timing)
{
  double result; // xmm0_8

  if ( *((_DWORD *)timing + 6) != 0 )
  {
    __asm
    {
      vmovsd  xmm0, qword ptr [rdi+10h]
      vcvtsi2sd xmm1, xmm1, eax
      vdivsd  xmm0, xmm0, xmm1
    }
  }
  else
  {
    __asm { vxorpd  xmm0, xmm0, xmm0 }
  }
  __asm
  {
    vcvtsi2sd xmm1, xmm2, dword ptr [rdi+60h]
    vmulsd  xmm1, xmm1, qword ptr [rax]
    vdivsd  xmm0, xmm0, xmm1
    vmulsd  xmm0, xmm0, cs:qword_125D220
  }
  *(_QWORD *)&result = _XMM0;
  return result;
}
