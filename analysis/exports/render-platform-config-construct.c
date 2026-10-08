// Constructs one zeroed 128-byte renderer platform-configuration slot.
void __fastcall render_platform_config_construct(__int64 a1, __m128 _XMM0)
{
  _RBX = a1;
  __asm { vxorps  xmm0, xmm0, xmm0 }
  __asm { vmovups xmmword ptr [rbx], xmm0 }
  *(_QWORD *)(a1 + 16) = 0;
  *(double *)&_XMM0 = nullsub_18(a1 + 24, "EASTL vector");
  __asm { vxorps  xmm0, xmm0, xmm0 }
  *(_QWORD *)(_RBX + 32) = -1;
  *(_DWORD *)(_RBX + 40) = 0;
  *(_BYTE *)(_RBX + 88) = 0;
  *(_QWORD *)(_RBX + 96) = 0x2000;
  *(_DWORD *)(_RBX + 104) = 0;
  __asm
  {
    vmovups xmmword ptr [rbx+70h], xmm0
    vxorps  ymm0, ymm0, ymm0
    vmovups ymmword ptr [rbx+30h], ymm0
  }
  *(_QWORD *)(_RBX + 80) = 0;
}
