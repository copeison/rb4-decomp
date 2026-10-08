/* Generated Hex-Rays evidence for the common render occlusion-query lifecycle. */

/* 0x5F7D30 */
__int64 __fastcall render_create_occlusion_query(__int64 a1)
{
  return (*(__int64 (__fastcall **)(_QWORD, __int64))(**(_QWORD **)(g_render_system + 304) + 120LL))(
           *(_QWORD *)(g_render_system + 304),
           a1);
}


/* 0x5F7D50 */
__int64 __fastcall render_occlusion_query_construct(__int64 _RDI, __int64 a2)
{
  _RCX = &unk_1B5D250;
  *(_QWORD *)_RDI = &unk_192AFE8;
  *(_QWORD *)(_RDI + 8) = a2;
  *(_BYTE *)(_RDI + 16) = 0;
  __asm
  {
    vmovups xmm0, xmmword ptr [rcx]
    vmovups xmmword ptr [rdi+14h], xmm0
  }
  *(_DWORD *)(_RDI + 36) = -1;
  *(_DWORD *)(_RDI + 40) = 0;
  *(_QWORD *)(_RDI + 56) = _RDI + 48;
  *(_QWORD *)(_RDI + 48) = _RDI + 48;
  *(_WORD *)(_RDI + 17) = 0;
  return _RDI + 48;
}


/* 0x5F7DA0 */
__int64 __fastcall render_occlusion_query_destruct(_QWORD *a1)
{
  __int64 result; // rax

  *a1 = &unk_192AFE8;
  result = a1[6];
  *(_QWORD *)(result + 8) = a1[7];
  *(_QWORD *)a1[7] = result;
  return result;
}


/* 0x5F7DD0 */
double __fastcall render_occlusion_query_delete(_QWORD *a1)
{
  __int64 v1; // rax

  *a1 = &unk_192AFE8;
  v1 = a1[6];
  *(_QWORD *)(v1 + 8) = a1[7];
  *(_QWORD *)a1[7] = v1;
  return sub_37BF50(a1);
}
