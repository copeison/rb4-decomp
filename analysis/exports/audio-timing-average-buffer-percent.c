// Returns mean elapsed time as a percentage of one audio buffer duration.
double __fastcall audio_timing_average_buffer_percent(const void *timing)
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
    vdivsd  xmm0, xmm0, qword ptr [rax]
    vmulsd  xmm0, xmm0, cs:qword_125D210
  }
  *(_QWORD *)&result = _XMM0;
  return result;
}
