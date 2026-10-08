/* Generated Hex-Rays evidence for the common render 1D-texture lifecycle. */

/* 0x6F5870 */
__int64 __fastcall render_texture_1d_construct(_QWORD *a1, __int64 a2)
{
  unsigned __int8 v4; // al
  __int64 result; // rax

  _R14 = a2;
  _RBX = a1;
  render_texture_construct(a1);
  *_RBX = &unk_1939C98;
  v4 = sub_69B990(_R14);
  __asm { vmovups ymm0, ymmword ptr [r14+70h] }
  __asm
  {
    vmovups ymmword ptr [rbx+118h], ymm0
    vmovups ymm0, ymmword ptr [r14]
    vmovups ymm1, ymmword ptr [r14+20h]
    vmovups ymm2, ymmword ptr [r14+40h]
    vmovups ymm3, ymmword ptr [r14+60h]
    vmovups ymmword ptr [rbx+108h], ymm3
    vmovups ymmword ptr [rbx+0E8h], ymm2
    vmovups ymmword ptr [rbx+0C8h], ymm1
    vmovups ymmword ptr [rbx+0A8h], ymm0
  }
  sub_682960(_RBX + 39, _R14 + 144, v4);
  *((_DWORD *)_RBX + 65) = *(_DWORD *)(_R14 + 164);
  *((_DWORD *)_RBX + 68) = *(_DWORD *)(_R14 + 160);
  result = *(_QWORD *)(_R14 + 152);
  _RBX[33] = result;
  __asm
  {
    vmovups ymm0, ymmword ptr [rbx+118h]
    vmovups ymmword ptr [rbx+80h], ymm0
    vmovups ymm0, ymmword ptr [rbx+0A8h]
    vmovups ymm1, ymmword ptr [rbx+0C8h]
    vmovups ymm2, ymmword ptr [rbx+0E8h]
    vmovups ymm3, ymmword ptr [rbx+108h]
    vmovups ymmword ptr [rbx+70h], ymm3
    vmovups ymmword ptr [rbx+50h], ymm2
    vmovups ymmword ptr [rbx+30h], ymm1
    vmovups ymmword ptr [rbx+10h], ymm0
  }
  return result;
}


/* 0x6F5970 */
__int64 __fastcall render_texture_1d_destruct(_QWORD *a1, __int64 a2, __m128 a3)
{
  *a1 = &unk_1939C98;
  sub_682BC0(a1 + 39, a2, a3);
  return render_texture_destruct((__int64)a1);
}


/* 0x6F59B0 */
double __fastcall render_texture_1d_delete(_QWORD *a1, __int64 a2, __m128 a3)
{
  *a1 = &unk_1939C98;
  sub_682BC0(a1 + 39, a2, a3);
  render_texture_destruct((__int64)a1);
  return sub_37BF50(a1);
}
