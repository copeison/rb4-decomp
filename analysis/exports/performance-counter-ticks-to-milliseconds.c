// Converts TSC ticks to milliseconds using 1000.0 / sceKernelGetTscFrequency().
double __fastcall performance_counter_ticks_to_milliseconds(unsigned __int64 ticks)
{
  double result; // xmm0_8

  __asm
  {
    vmovq   xmm0, rdi
    vpunpckldq xmm0, xmm0, cs:xmmword_125B040
    vsubpd  xmm0, xmm0, cs:xmmword_125B050
    vhaddpd xmm0, xmm0, xmm0
    vmulsd  xmm0, xmm0, cs:qword_19F26A0
  }
  *(_QWORD *)&result = _XMM0;
  return result;
}
