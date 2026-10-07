__int64 __fastcall fmod_studio_sound_generator_configure_fade(
        __int64 a1,
        int a2,
        __m128 _XMM0,
        __m128 _XMM1,
        double a5,
        __m128 _XMM3)
{
  __int64 result; // rax

  __asm
  {
    vmulss  xmm2, xmm1, cs:dword_125CF6C
    vxorps  xmm3, xmm3, xmm3
  }
  _RBX = a1;
  __asm
  {
    vmaxss  xmm2, xmm2, dword ptr [rax]
    vucomiss xmm2, xmm3
  }
  __asm { vmovss  dword ptr [rbx+5Ch], xmm2 }
  *(_BYTE *)(a1 + 104) = 1;
  __asm { vxorps  xmm2, xmm2, xmm2 }
  result = *(unsigned int *)(a1 + 84);
  *(_DWORD *)(a1 + 80) = result;
  __asm { vmovss  dword ptr [rbx+58h], xmm0 }
  *(_DWORD *)(a1 + 96) = 0;
  *(_DWORD *)(a1 + 100) = 0;
  *(_BYTE *)(a1 + 104) = 0;
  __asm
  {
    vmovups xmmword ptr [rbx+70h], xmm2
    vxorps  xmm2, xmm2, xmm2
    vucomiss xmm1, xmm2
  }
  *(_BYTE *)(a1 + 128) = a2 == 1;
  return result;
}
