// Constructs the default render-resource state; initializes light mode to directional and scale to 100.0.
__int64 __fastcall render_construct_default_resources(_QWORD *a1)
{
  _RBX = a1;
  __asm { vxorps  ymm0, ymm0, ymm0 }
  *a1 = 0;
  __asm
  {
    vmovups ymmword ptr [rbx+1E0h], ymm0
    vmovups ymmword ptr [rbx+1C0h], ymm0
    vmovups ymmword ptr [rbx+1A0h], ymm0
  }
  a1[64] = 0;
  *(double *)&_XMM0 = nullsub_18(a1 + 65, "EASTL vector");
  __asm { vxorps  xmm0, xmm0, xmm0 }
  __asm { vmovups xmmword ptr [rbx+210h], xmm0 }
  _RBX[68] = 0;
  nullsub_18(_RBX + 69, "EASTL vector");
  __asm { vxorps  ymm0, ymm0, ymm0 }
  _RBX[70] = 0x42C8000000000000LL;
  __asm
  {
    vmovups ymmword ptr [rbx+180h], ymm0
    vmovups ymmword ptr [rbx+168h], ymm0
    vmovups ymmword ptr [rbx+148h], ymm0
    vmovups ymmword ptr [rbx+128h], ymm0
    vmovups ymmword ptr [rbx+108h], ymm0
    vmovups ymmword ptr [rbx+0E8h], ymm0
    vmovups ymmword ptr [rbx+0C8h], ymm0
    vmovups ymmword ptr [rbx+0A8h], ymm0
    vmovups ymmword ptr [rbx+88h], ymm0
    vmovups ymmword ptr [rbx+68h], ymm0
    vmovups ymmword ptr [rbx+48h], ymm0
    vmovups ymmword ptr [rbx+28h], ymm0
    vmovups ymmword ptr [rbx+8], ymm0
  }
  return 0x42C8000000000000LL;
}
