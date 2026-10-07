void __fastcall audio_clip_fmod_prepare_for_audio_reset(__int64 _RDI, double a2, __m128 _XMM1, __m128 _XMM2)
{
  bool v5; // zf

  if ( *(_DWORD *)(_RDI + 28) != 5 )
  {
    if ( *(_DWORD *)(_RDI + 320) == 1 )
    {
      _RAX = *(_QWORD *)(_RDI + 256);
      v5 = _RAX == 0;
      if ( _RAX != 0 )
      {
        __asm { vmovsd  xmm0, qword ptr [rax+98h] }
        __asm
        {
          vcvtsd2ss xmm0, xmm0, xmm0
          vcvtsi2ss xmm1, xmm1, rax
          vdivss  xmm0, xmm0, xmm1
        }
      }
      else
      {
        _RAX = &unk_19B02A8;
        __asm { vmovss  xmm0, dword ptr [rax] }
      }
      _RAX = &dword_124D444;
      __asm
      {
        vxorps  xmm2, xmm2, xmm2
        vmovss  dword ptr [rdi+124h], xmm0
        vmovss  xmm1, dword ptr [rax]
        vucomiss xmm2, xmm1
      }
      if ( v5 )
      {
        __asm
        {
          vmovss  xmm0, cs:dword_125CC90
          vmovss  dword ptr [rdi+11Ch], xmm0
        }
      }
      else
      {
        __asm
        {
          vmovss  xmm2, cs:dword_125CC94
          vmulss  xmm0, xmm1, xmm0
          vdivss  xmm0, xmm2, xmm0
          vmovss  dword ptr [rdi+11Ch], xmm0
        }
      }
    }
    else
    {
      *(_DWORD *)(_RDI + 28) = 6;
    }
  }
}
