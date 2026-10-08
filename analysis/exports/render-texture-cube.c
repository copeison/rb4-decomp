// render_texture_cube_construct @ 0x6A0DA0
__int64 __fastcall render_texture_cube_construct(_QWORD *a1, __int64 a2)
{
  bool v4; // al
  double v10; // xmm0_8
  __int64 result; // rax

  _R14 = a2;
  _RBX = a1;
  render_texture_construct(a1);
  *_RBX = &unk_1935CE0;
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
  sub_68CC00(_RBX + 39, _R14 + 144, v4, v10);
  *((_DWORD *)_RBX + 40) = 2;
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


// render_texture_cube_destruct @ 0x6A0EA0
__int64 __fastcall render_texture_cube_destruct(_QWORD *a1, __int64 a2, __m128 a3)
{
  __int64 v4; // rsi
  __m128 v5; // xmm0
  __int64 v6; // rsi
  __m128 v7; // xmm0
  __int64 v8; // rsi
  __m128 v9; // xmm0
  __int64 v10; // rsi
  __m128 v11; // xmm0
  __int64 v12; // rsi
  __m128 v13; // xmm0

  *a1 = &unk_1935CE0;
  sub_682BC0(a1 + 89, a2, a3);
  sub_682BC0(a1 + 79, v4, v5);
  sub_682BC0(a1 + 69, v6, v7);
  sub_682BC0(a1 + 59, v8, v9);
  sub_682BC0(a1 + 49, v10, v11);
  sub_682BC0(a1 + 39, v12, v13);
  return render_texture_destruct((__int64)a1);
}


// render_texture_cube_delete @ 0x6A0F10
double __fastcall render_texture_cube_delete(_QWORD *a1, __int64 a2, __m128 a3)
{
  __int64 v4; // rsi
  __m128 v5; // xmm0
  __int64 v6; // rsi
  __m128 v7; // xmm0
  __int64 v8; // rsi
  __m128 v9; // xmm0
  __int64 v10; // rsi
  __m128 v11; // xmm0
  __int64 v12; // rsi
  __m128 v13; // xmm0

  *a1 = &unk_1935CE0;
  sub_682BC0(a1 + 89, a2, a3);
  sub_682BC0(a1 + 79, v4, v5);
  sub_682BC0(a1 + 69, v6, v7);
  sub_682BC0(a1 + 59, v8, v9);
  sub_682BC0(a1 + 49, v10, v11);
  sub_682BC0(a1 + 39, v12, v13);
  render_texture_destruct((__int64)a1);
  return sub_37BF50(a1);
}

