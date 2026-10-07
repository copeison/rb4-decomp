__int128 __usercall fmod_buffered_stream_interpolate_optimal32_6p5o@<xmm0>(__int64 a1@<rdi>, __int64 _RSI@<rsi>)
{
  __int128 result; // xmm0

  __asm
  {
    vmovss  xmm3, dword ptr [rsi+0Ch]; Float-rounded coefficients match Olli Niemitalo's Optimal 32x, 6-point, 5th-order z-form audio interpolator.
    vmovss  xmm4, dword ptr [rsi+10h]
    vmovss  xmm5, dword ptr [rsi+14h]
    vmovss  xmm6, dword ptr [rsi]
    vmovsd  xmm7, qword ptr [rsi+4]
    vsubss  xmm1, xmm3, dword ptr [rsi+8]
    vsubss  xmm2, xmm4, dword ptr [rsi+4]
    vinsertps xmm3, xmm4, xmm3, 10h
    vinsertps xmm7, xmm7, xmm6, 20h ; ' '
    vinsertps xmm3, xmm3, xmm5, 20h ; ' '
    vinsertps xmm3, xmm3, cs:dword_125CDBC, 30h ; '0'
    vinsertps xmm0, xmm7, xmm0, 30h ; '0'
    vmulss  xmm4, xmm2, cs:dword_125CDC0
    vmulss  xmm7, xmm1, cs:dword_125CDD8
    vaddps  xmm0, xmm0, xmm3
    vsubss  xmm3, xmm5, xmm6
    vmulss  xmm5, xmm1, cs:dword_125CDC4
    vmulss  xmm6, xmm0, cs:dword_125CDD0
    vinsertps xmm1, xmm2, xmm1, 10h
    vinsertps xmm1, xmm1, xmm3, 20h ; ' '
    vblendps xmm1, xmm1, xmm0, 8
    vaddss  xmm4, xmm4, xmm5
    vmulss  xmm5, xmm3, cs:dword_125CDC8
    vaddss  xmm8, xmm4, xmm5
    vmulss  xmm4, xmm2, cs:dword_125CDDC
    vmovshdup xmm5, xmm0
    vmulss  xmm5, xmm5, cs:dword_125CDCC
    vaddss  xmm4, xmm4, xmm7
    vmulss  xmm7, xmm3, cs:dword_125CDE0
    vaddss  xmm5, xmm6, xmm5
    vpermilpd xmm6, xmm0, 1
    vmulss  xmm6, xmm6, cs:dword_125CDD4
    vaddss  xmm4, xmm4, xmm7
    vpermilps xmm7, xmm0, 0E7h
    vmulss  xmm4, xmm4, xmm7
    vaddss  xmm4, xmm6, xmm4
    vaddss  xmm4, xmm5, xmm4
    vmovss  xmm5, cs:dword_125CDE4
    vinsertps xmm5, xmm5, cs:dword_125CDE8, 10h
    vmulss  xmm4, xmm4, xmm7
    vinsertps xmm5, xmm5, cs:dword_125CDEC, 20h ; ' '
    vaddss  xmm4, xmm8, xmm4
    vinsertps xmm4, xmm5, xmm4, 30h ; '0'
    vmulps  xmm4, xmm4, xmm0
    vpermilpd xmm5, xmm4, 1
    vaddps  xmm4, xmm4, xmm5
    vmovups xmm5, cs:xmmword_125CE10
    vhaddps xmm4, xmm4, xmm4
    vinsertps xmm4, xmm5, xmm4, 30h ; '0'
    vmulps  xmm1, xmm4, xmm1
    vpermilpd xmm2, xmm1, 1
    vaddps  xmm1, xmm1, xmm2
    vmovups xmm2, cs:xmmword_125CE20
    vhaddps xmm1, xmm1, xmm1
    vinsertps xmm1, xmm2, xmm1, 30h ; '0'
    vmulps  xmm0, xmm1, xmm0
    vpermilpd xmm1, xmm0, 1
    vaddps  xmm0, xmm0, xmm1
    vhaddps xmm0, xmm0, xmm0
  }
  return result;
}
