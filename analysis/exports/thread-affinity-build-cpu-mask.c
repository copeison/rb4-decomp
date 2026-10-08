__int64 __fastcall thread_affinity_build_cpu_mask(
        __int64 a1,
        __int64 a2,
        double a3,
        double a4,
        __m128 _XMM2,
        __m128 _XMM3,
        double a7,
        double a8,
        double a9,
        __m128 _XMM7)
{
  __int64 v11; // rax
  __int64 result; // rax
  unsigned __int64 v13; // rcx
  unsigned __int64 v25; // rax
  bool v32; // zf
  __int64 v56; // rsi

  v11 = 0;
  if ( a1 != -1 )
    v11 = 1LL << a1;
  result = a2 | v11;
  if ( result == 0 )
  {
    if ( unk_19E8818 == 0 )
      return 0;
    if ( unk_19E8818 >= 8u && (v13 = unk_19E8818 & 0xFFFFFFFFFFFFFFF8LL, (unk_19E8818 & 0xFFFFFFFFFFFFFFF8LL) != 0) )
    {
      __asm
      {
        vmovdqa xmm8, cs:xmmword_125AD00
        vmovdqa xmm9, cs:xmmword_125AD10
        vmovdqa xmm10, cs:xmmword_125AD20
        vmovdqu xmm12, cs:xmmword_125AD30
        vmovdqa xmm11, cs:xmmword_125AD40
      }
      _RAX = 1;
      __asm
      {
        vpxor   xmm14, xmm14, xmm14
        vpxor   xmm7, xmm7, xmm7
        vpxor   xmm2, xmm2, xmm2
        vpxor   xmm3, xmm3, xmm3
        vmovq   xmm0, rax
      }
      v25 = unk_19E8818 & 0xFFFFFFFFFFFFFFF8LL;
      __asm { vpslldq xmm1, xmm0, 8 }
      do
      {
        __asm
        {
          vpshufd xmm0, xmm1, 4Eh ; 'N'
          vpaddq  xmm4, xmm1, xmm8
          vpsllq  xmm13, xmm12, xmm1
          vpaddq  xmm5, xmm1, xmm10
          vpaddq  xmm6, xmm1, xmm9
          vpaddq  xmm1, xmm1, xmm11
        }
        v32 = v25 == 8;
        v25 -= 8LL;
        __asm
        {
          vpsllq  xmm0, xmm12, xmm0
          vpsllq  xmm15, xmm12, xmm5
          vpshufd xmm5, xmm5, 4Eh ; 'N'
          vpblendw xmm13, xmm13, xmm0, 0F0h
          vpsllq  xmm0, xmm12, xmm4
          vpshufd xmm4, xmm4, 4Eh ; 'N'
          vpsllq  xmm5, xmm12, xmm5
          vpsllq  xmm4, xmm12, xmm4
          vpblendw xmm5, xmm15, xmm5, 0F0h
          vpor    xmm14, xmm13, xmm14
          vpblendw xmm0, xmm0, xmm4, 0F0h
          vpsllq  xmm4, xmm12, xmm6
          vpshufd xmm6, xmm6, 4Eh ; 'N'
          vpor    xmm3, xmm5, xmm3
          vpsllq  xmm6, xmm12, xmm6
          vpor    xmm7, xmm0, xmm7
          vpblendw xmm4, xmm4, xmm6, 0F0h
          vpor    xmm2, xmm4, xmm2
        }
      }
      while ( !v32 );
      __asm
      {
        vpor    xmm0, xmm7, xmm14
        vpor    xmm0, xmm2, xmm0
        vpor    xmm0, xmm3, xmm0
        vpshufd xmm1, xmm0, 4Eh ; 'N'
        vpor    xmm0, xmm0, xmm1
        vmovq   rax, xmm0
      }
      if ( unk_19E8818 == v13 )
        return result;
    }
    else
    {
      v13 = 0;
      result = 0;
    }
    do
    {
      v56 = 1LL << v13++;
      result |= v56;
    }
    while ( unk_19E8818 != v13 );
  }
  return result;
}
