/* Generated Hex-Rays evidence for the common render-texture lifecycle. */

/* 0x69B6E0 */
void *__fastcall render_texture_construct(_QWORD *a1)
{
  _RBX = a1;
  sub_6427D0(a1);
  __asm { vxorps  ymm0, ymm0, ymm0 }
  _ECX = -1;
  *_RBX = &unk_1935688;
  *((_DWORD *)_RBX + 4) = -1;
  __asm
  {
    vmovups ymmword ptr [rbx+4Ch], ymm0
    vmovups ymmword ptr [rbx+34h], ymm0
    vmovups ymmword ptr [rbx+14h], ymm0
    vmovd   xmm0, ecx
    vmovdqu xmmword ptr [rbx+6Ch], xmm0
  }
  _RBX[16] = 0;
  *((_DWORD *)_RBX + 34) = 0;
  *((_BYTE *)_RBX + 140) = 0;
  *((_DWORD *)_RBX + 36) = -1;
  *((_DWORD *)_RBX + 37) = 0;
  _RBX[19] = 0;
  *((_DWORD *)_RBX + 40) = -1;
  return &unk_1935688;
}


/* 0x69B770 */
// attributes: thunk
__int64 __fastcall render_texture_destruct(__int64 a1)
{
  return nullsub_49(a1);
}


/* 0x69B780 */
double __fastcall render_texture_delete(__int64 a1)
{
  nullsub_49(a1);
  return sub_37BF50(a1);
}
