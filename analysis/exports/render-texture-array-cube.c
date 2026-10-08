/* Generated Hex-Rays evidence for the common render cube texture-array lifecycle. */

/* 0x69AAC0 */
__int64 __fastcall render_texture_array_cube_construct(_QWORD *a1, __int64 a2)
{
  unsigned __int8 v4; // al
  __m128 v5; // xmm0
  __int64 v6; // rax
  __int64 result; // rax

  _RBX = a1;
  render_texture_construct(a1);
  *_RBX = &unk_19355D0;
  v4 = sub_69B990(a2);
  sub_69AF50(_RBX + 21, a2, v4, v5);
  sub_69ABB0(_RBX + 21);
  *((_DWORD *)_RBX + 40) = 2;
  v6 = *(_QWORD *)(a2 + 144);
  *((_DWORD *)_RBX + 65) = *(_DWORD *)(v6 + 20);
  *((_DWORD *)_RBX + 68) = *(_DWORD *)(v6 + 16);
  _RBX[33] = *(_QWORD *)(v6 + 8);
  result = (__int64)(*(_QWORD *)(a2 + 152) - *(_QWORD *)(a2 + 144)) >> 5;
  _RBX[35] = 0xEEEEEEEEEEEEEEEFLL * result;
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


/* 0x69ACD0 */
__int64 __fastcall render_texture_array_cube_destruct(_QWORD *a1)
{
  *a1 = &unk_19355D0;
  sub_48D3D0(a1 + 39);
  return render_texture_destruct((__int64)a1);
}


/* 0x69AD10 */
double __fastcall render_texture_array_cube_delete(_QWORD *a1)
{
  *a1 = &unk_19355D0;
  sub_48D3D0(a1 + 39);
  render_texture_destruct((__int64)a1);
  return sub_37BF50(a1);
}
