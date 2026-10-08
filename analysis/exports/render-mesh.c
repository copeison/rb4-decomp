// render_mesh_construct @ 0x5C2700
__int64 __fastcall render_mesh_construct(_QWORD *a1, __int64 a2)
{
  __int64 result; // rax

  _RBX = a1;
  *a1 = &unk_1928888;
  sub_6C9BA0(a1 + 1);
  _RAX = &unk_19287F0;
  _RCX = &unk_1928860;
  __asm
  {
    vmovq   xmm1, rax
    vmovq   xmm0, rcx
    vpunpcklqdq xmm0, xmm1, xmm0
    vmovdqu xmmword ptr [rbx], xmm0
    vpxor   xmm0, xmm0, xmm0
    vmovdqu xmmword ptr [rbx+18h], xmm0
  }
  _RBX[5] = 0;
  *(double *)&_XMM0 = nullsub_18(_RBX + 6, "EASTL vector");
  _RAX = &unk_1B5D250;
  __asm
  {
    vpxor   xmm0, xmm0, xmm0
    vmovdqu xmmword ptr [rbx+38h], xmm0
  }
  _RBX[9] = -1;
  *((_BYTE *)_RBX + 80) = 0;
  *((_BYTE *)_RBX + 81) = 0;
  *(_QWORD *)((char *)_RBX + 84) = 0;
  __asm { vmovups xmm0, xmmword ptr [rax] }
  __asm { vmovups xmmword ptr [rbx+5Ch], xmm0 }
  result = (unsigned int)_InterlockedExchange((volatile __int32 *)_RBX + 27, 0);
  _RBX[14] = -1;
  _RBX[15] = a2;
  return result;
}


// render_mesh_destruct @ 0x5C27B0
double __fastcall render_mesh_destruct(_QWORD *_RDI)
{
  _QWORD *v1; // rbx
  __int64 v7; // rsi

  v1 = _RDI + 1;
  _RAX = &unk_19287F0;
  _RCX = &unk_1928860;
  __asm
  {
    vmovq   xmm1, rax
    vmovq   xmm0, rcx
    vpunpcklqdq xmm0, xmm1, xmm0
    vmovdqu xmmword ptr [rdi], xmm0
  }
  v7 = _RDI[3];
  if ( v7 != 0 )
    sub_252D30(_RDI + 6, v7, _RDI[5] - v7);
  return sub_6C9BC0(v1);
}


// render_mesh_secondary_destruct @ 0x5C2810
double __fastcall render_mesh_secondary_destruct(_QWORD *a1)
{
  __int64 v1; // rax
  __int64 v8; // rsi
  __int64 savedregs; // [rsp+8h] [rbp+0h]

  savedregs = v1;
  _RBX = a1;
  _RAX = &unk_19287F0;
  _RCX = &unk_1928860;
  __asm
  {
    vmovq   xmm1, rax
    vmovq   xmm0, rcx
    vpunpcklqdq xmm0, xmm1, xmm0
    vmovdqu xmmword ptr [rbx-8], xmm0
  }
  v8 = a1[2];
  if ( v8 != 0 )
    sub_252D30(a1 + 5, v8, a1[4] - v8);
  return sub_6C9BC0(_RBX);
}


// render_mesh_delete @ 0x5C2870
double __fastcall render_mesh_delete(_QWORD *a1)
{
  _QWORD *v2; // r14
  __int64 v8; // rsi

  _RBX = a1;
  v2 = a1 + 1;
  _RAX = &unk_19287F0;
  _RCX = &unk_1928860;
  __asm
  {
    vmovq   xmm1, rax
    vmovq   xmm0, rcx
    vpunpcklqdq xmm0, xmm1, xmm0
    vmovdqu xmmword ptr [rbx], xmm0
  }
  v8 = a1[3];
  if ( v8 != 0 )
    sub_252D30(a1 + 6, v8, a1[5] - v8);
  sub_6C9BC0(v2);
  return sub_37BF50(_RBX);
}


// render_mesh_secondary_delete @ 0x5C28D0
double __fastcall render_mesh_secondary_delete(_QWORD *a1)
{
  __int64 v8; // rsi

  _R14 = a1 - 1;
  _RAX = &unk_19287F0;
  _RCX = &unk_1928860;
  __asm
  {
    vmovq   xmm1, rax
    vmovq   xmm0, rcx
    vpunpcklqdq xmm0, xmm1, xmm0
    vmovdqu xmmword ptr [rbx-8], xmm0
  }
  v8 = a1[2];
  if ( v8 != 0 )
    sub_252D30(_R14 + 6, v8, _R14[5] - v8);
  sub_6C9BC0(a1);
  return sub_37BF50(_R14);
}

