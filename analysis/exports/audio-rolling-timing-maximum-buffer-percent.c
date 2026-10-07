// Returns maximum rolling-window time normalized by window size and audio-buffer duration.
double __fastcall audio_rolling_timing_maximum_buffer_percent(const void *timing)
{
  double result; // xmm0_8

  __asm { vmovsd  xmm0, qword ptr [rdi+20h] }
  __asm
  {
    vcvtsi2sd xmm1, xmm1, dword ptr [rdi+60h]
    vmulsd  xmm1, xmm1, qword ptr [rax]
    vdivsd  xmm0, xmm0, xmm1
    vmulsd  xmm0, xmm0, cs:qword_125D228
  }
  *(_QWORD *)&result = _XMM0;
  return result;
}
