__int128 __usercall audio_get_sample_rate@<xmm0>()
{
  __int128 result; // xmm0

  _RAX = &unk_19B02B0;
  __asm { vmovsd  xmm0, qword ptr [rax] }
  return result;
}
