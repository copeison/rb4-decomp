// render_texture_2d_construct @ 0x6900B0
__int64 __fastcall render_texture_2d_construct(_QWORD *a1, __int64 a2)
{
  unsigned __int8 v4; // al
  __int64 result; // rax

  _R14 = a2;
  _RBX = a1;
  render_texture_construct(a1);
  *_RBX = &unk_19349D0;
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
  _RBX[49] = 0;
  _RBX[50] = -1;
  *((_DWORD *)_RBX + 40) = 0;
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


// render_texture_2d_destruct @ 0x6901D0
__int64 __fastcall render_texture_2d_destruct(_QWORD *a1, __int64 a2, __m128 a3)
{
  *a1 = &unk_19349D0;
  sub_682BC0(a1 + 39, a2, a3);
  return render_texture_destruct((__int64)a1);
}


// render_texture_2d_delete @ 0x690210
double __fastcall render_texture_2d_delete(_QWORD *a1, __int64 a2, __m128 a3)
{
  __int64 v3; // rax
  __int64 savedregs; // [rsp+8h] [rbp+0h]

  savedregs = v3;
  *a1 = &unk_19349D0;
  sub_682BC0(a1 + 39, a2, a3);
  render_texture_destruct((__int64)a1);
  return sub_37BF50(a1);
}


// render_texture_2d_set_linked_resource @ 0x690250
void __fastcall render_texture_2d_set_linked_resource(__int64 a1, __int64 a2, __int64 a3)
{
  *(_QWORD *)(a1 + 392) = a2;
  *(_QWORD *)(a1 + 400) = a3;
}

