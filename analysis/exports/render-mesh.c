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


// render_mesh_set_vertices_resident @ 0x5C2930
void __fastcall render_mesh_set_vertices_resident(__int64 a1, char a2)
{
  *(_BYTE *)(a1 + 80) = a2;
}


// render_mesh_set_vertex_usage_flags @ 0x5C2940
void __fastcall render_mesh_set_vertex_usage_flags(__int64 a1, int a2)
{
  *(_DWORD *)(a1 + 84) = a2;
}


// render_mesh_set_triangle_usage_flags @ 0x5C2950
void __fastcall render_mesh_set_triangle_usage_flags(__int64 a1, int a2)
{
  *(_DWORD *)(a1 + 88) = a2;
}


// render_mesh_requires_vertex_storage @ 0x5C2960
bool __fastcall render_mesh_requires_vertex_storage(_BYTE *a1)
{
  bool result; // al

  result = true;
  if ( a1[80] == 0 && a1[81] == 0 )
    return (a1[84] & 5) != 0;
  return result;
}


// render_mesh_requires_triangle_storage @ 0x5C2980
bool __fastcall render_mesh_requires_triangle_storage(_BYTE *a1)
{
  bool result; // al

  result = true;
  if ( a1[80] == 0 && a1[81] == 0 )
    return (a1[88] & 5) != 0;
  return result;
}


// render_mesh_finalize @ 0x5C29A0
__int64 __fastcall render_mesh_finalize(_BYTE *a1)
{
  __int128 v5; // [rsp+0h] [rbp-40h] BYREF
  __int64 v6; // [rsp+10h] [rbp-30h]
  _BYTE v7[8]; // [rsp+18h] [rbp-28h] BYREF
  __int64 v8; // [rsp+20h] [rbp-20h]

  v8 = 0x6365786562696C2FLL;
  *((_QWORD *)a1 + 8) = 0xAAAAAAAAAAAAAAABLL * ((__int64)(*((_QWORD *)a1 + 4) - *((_QWORD *)a1 + 3)) >> 2);
  (*(void (__fastcall **)(_BYTE *))(*(_QWORD *)a1 + 72LL))(a1);
  if ( *((_WORD *)a1 + 40) == 0 )
  {
    if ( (a1[84] & 5) != 0 || ((*(void (__fastcall **)(_BYTE *))(*(_QWORD *)a1 + 48LL))(a1), a1[80] == 0) )
    {
      if ( a1[81] == 0 && (a1[88] & 5) == 0 )
      {
        __asm { vxorps  xmm0, xmm0, xmm0 }
        __asm { vmovups [rbp+var_40], xmm0 }
        v6 = 0;
        nullsub_18(v7, "EASTL vector");
        sub_5C2A80((__int64 *)a1 + 3, (__int64 *)&v5);
        if ( (_QWORD)v5 != 0 )
          sub_252D30(v7, v5, v6 - v5);
      }
    }
  }
  return 0x6365786562696C2FLL;
}


// render_mesh_apply_updates @ 0x5C2DF0
__int64 __fastcall render_mesh_apply_updates(_QWORD *a1, __int64 a2, char a3)
{
  a1[14] = *(_QWORD *)(g_render_system + 160);
  if ( (a3 & 2) != 0 )
    a1[8] = 0xAAAAAAAAAAAAAAABLL * ((__int64)(a1[4] - a1[3]) >> 2);
  return (*(__int64 (__fastcall **)(_QWORD *, __int64))(*a1 + 80LL))(a1, a2);
}


// render_mesh_process_pending_updates @ 0x5C2E40
// Processes mesh dirty flags, updates counts, invokes virtual backend update, then clears the flags.
__int64 __fastcall render_mesh_process_pending_updates(__int64 a1)
{
  __int64 result; // rax
  int v3; // edx

  result = *(unsigned int *)(a1 + 108);
  if ( (_DWORD)result != 0 )
  {
    v3 = *(_DWORD *)(a1 + 108);
    *(_QWORD *)(a1 + 112) = *(_QWORD *)(g_render_system + 160);
    if ( (v3 & 2) != 0 )
      *(_QWORD *)(a1 + 64) = 0xAAAAAAAAAAAAAAABLL * ((__int64)(*(_QWORD *)(a1 + 32) - *(_QWORD *)(a1 + 24)) >> 2);
    (*(void (__fastcall **)(__int64))(*(_QWORD *)a1 + 80LL))(a1);
    return (unsigned int)_InterlockedExchange((volatile __int32 *)(a1 + 108), 0);
  }
  return result;
}


// render_mesh_process_pending_updates_secondary @ 0x5C2EA0
__int64 __fastcall render_mesh_process_pending_updates_secondary(__int64 a1)
{
  __int64 result; // rax
  __int64 v2; // rbx
  int v3; // edx

  result = *(unsigned int *)(a1 + 100);
  if ( (_DWORD)result != 0 )
  {
    v2 = a1 - 8;
    v3 = *(_DWORD *)(a1 - 8 + 108);
    *(_QWORD *)(a1 - 8 + 112) = *(_QWORD *)(g_render_system + 160);
    if ( (v3 & 2) != 0 )
      *(_QWORD *)(v2 + 64) = 0xAAAAAAAAAAAAAAABLL * ((__int64)(*(_QWORD *)(v2 + 32) - *(_QWORD *)(v2 + 24)) >> 2);
    (*(void (__fastcall **)(__int64))(*(_QWORD *)v2 + 80LL))(v2);
    return (unsigned int)_InterlockedExchange((volatile __int32 *)(v2 + 108), 0);
  }
  return result;
}

