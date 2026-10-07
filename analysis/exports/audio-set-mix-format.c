// Updates global sample-rate, reciprocal, buffer length, buffers/second, and milliseconds/buffer values.
// local variable allocation has failed, the output may be wrong!
void __fastcall audio_set_mix_format(double sample_rate, int buffer_length)
{
  __asm { vmovsd  xmm1, cs:qword_124D420 }
  _RAX = &unk_19B02B0;
  __asm
  {
    vdivsd  xmm1, xmm1, xmm0
    vmovsd  qword ptr [rax], xmm0
  }
  _RAX = &unk_19C9940;
  __asm
  {
    vcvtsd2ss xmm0, xmm0, xmm0
    vmovsd  qword ptr [rax], xmm1
    vcvtsi2ss xmm1, xmm2, edi
  }
  __asm
  {
    vdivss  xmm0, xmm0, xmm1
    vmovsd  xmm1, cs:qword_124D428
  }
  unk_19B02A4 = buffer_length;
  _RAX = &unk_19B02A8;
  __asm
  {
    vmovss  dword ptr [rax], xmm0
    vcvtss2sd xmm0, xmm0, xmm0
  }
  _RAX = &unk_19C9938;
  __asm
  {
    vdivsd  xmm0, xmm1, xmm0
    vmovsd  qword ptr [rax], xmm0
  }
}
