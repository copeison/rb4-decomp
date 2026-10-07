// Build FMOD_3D_ATTRIBUTES from an engine transform, flipping the X axes and clearing velocity.
__int64 __fastcall audio_build_fmod_3d_attributes(_DWORD *_RDI, __int64 _RSI)
{
  __asm
  {
    vmovss  xmm0, dword ptr [rdi+24h]
    vmovups xmm1, cs:xmmword_125D370
    vxorps  xmm0, xmm0, xmm1
    vmovss  dword ptr [rsi], xmm0
  }
  *(_DWORD *)(_RSI + 4) = _RDI[10];
  *(_DWORD *)(_RSI + 8) = _RDI[11];
  __asm
  {
    vmovss  xmm0, dword ptr [rdi+0Ch]
    vxorps  xmm0, xmm0, xmm1
    vmovss  dword ptr [rsi+18h], xmm0
  }
  *(_DWORD *)(_RSI + 28) = _RDI[4];
  *(_DWORD *)(_RSI + 32) = _RDI[5];
  __asm
  {
    vmovss  xmm0, dword ptr [rdi+18h]
    vxorps  xmm0, xmm0, xmm1
    vmovss  dword ptr [rsi+24h], xmm0
  }
  *(_DWORD *)(_RSI + 40) = _RDI[7];
  *(_DWORD *)(_RSI + 44) = _RDI[8];
  *(_QWORD *)(_RSI + 12) = 0x80000000LL;
  *(_DWORD *)(_RSI + 20) = 0;
  return 0x80000000LL;
}
