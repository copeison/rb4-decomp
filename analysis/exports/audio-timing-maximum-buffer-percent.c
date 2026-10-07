// Returns maximum elapsed time as a percentage of one audio buffer duration.
double __fastcall audio_timing_maximum_buffer_percent(const void *timing)
{
  double result; // xmm0_8

  __asm
  {
    vmovsd  xmm0, qword ptr [rdi+20h]
    vdivsd  xmm0, xmm0, qword ptr [rax]
    vmulsd  xmm0, xmm0, cs:qword_125D218
  }
  *(_QWORD *)&result = _XMM0;
  return result;
}
