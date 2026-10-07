// Converts milliseconds to TSC ticks using the reciprocal established at startup.
// local variable allocation has failed, the output may be wrong!
unsigned __int64 __fastcall performance_counter_milliseconds_to_ticks(double milliseconds)
{
  char v1; // zf
  unsigned __int64 v10; // rcx

  __asm
  {
    vmovsd  xmm1, cs:qword_19F26A0
    vxorps  xmm2, xmm2, xmm2
    vucomisd xmm2, xmm1
  }
  if ( v1 )
    return 0;
  __asm
  {
    vdivsd  xmm0, xmm0, xmm1
    vmovsd  xmm1, cs:qword_125B068
  }
  __asm
  {
    vsubsd  xmm2, xmm0, xmm1
    vcvttsd2si rax, xmm2
  }
  v10 = _RAX ^ 0x8000000000000000LL;
  __asm
  {
    vcvttsd2si rax, xmm0
    vucomisd xmm0, xmm1
  }
  return v10;
}
