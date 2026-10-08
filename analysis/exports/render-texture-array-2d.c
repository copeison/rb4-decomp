/* Generated Hex-Rays evidence for the common render 2D texture-array lifecycle. */

/* 0x698160 */
__int64 __fastcall render_texture_array_2d_construct(_QWORD *a1, __int64 a2)
{
  double v11; // xmm0_8
  __int64 v13; // rbx
  __int64 i; // r14
  _QWORD *v15; // rdi
  double v17; // xmm0_8
  double v19; // xmm0_8
  __int64 v20; // rax
  __int64 v27; // [rsp+0h] [rbp-70h]
  __int64 v28; // [rsp+10h] [rbp-60h] BYREF
  void *v29; // [rsp+18h] [rbp-58h] BYREF
  __int128 v30; // [rsp+20h] [rbp-50h]
  int v31; // [rsp+30h] [rbp-40h]
  int v32; // [rsp+34h] [rbp-3Ch]
  __int64 v33; // [rsp+38h] [rbp-38h]
  __int64 v34; // [rsp+40h] [rbp-30h]

  _R14 = a2;
  _R13 = a1;
  v34 = 0x6365786562696C2FLL;
  render_texture_construct(a1);
  *_R13 = &unk_1935220;
  LOBYTE(v29) = sub_69B990(_R14);
  __asm
  {
    vmovups ymm0, ymmword ptr [r14+70h]
    vmovups ymmword ptr [r13+118h], ymm0
    vmovups ymm0, ymmword ptr [r14]
    vmovups ymm1, ymmword ptr [r14+20h]
    vmovups ymm2, ymmword ptr [r14+40h]
    vmovups ymm3, ymmword ptr [r14+60h]
    vmovups ymmword ptr [r13+108h], ymm3
    vmovups ymmword ptr [r13+0E8h], ymm2
    vmovups ymmword ptr [r13+0C8h], ymm1
    vmovups ymmword ptr [r13+0A8h], ymm0
    vxorps  xmm0, xmm0, xmm0
    vmovups xmmword ptr [r13+138h], xmm0
  }
  _R13[41] = 0;
  v11 = nullsub_18(_R13 + 42, "EASTL vector");
  sub_697470(_R13 + 39, 0xCCCCCCCCCCCCCCCDLL * ((__int64)(*(_QWORD *)(_R14 + 152) - *(_QWORD *)(_R14 + 144)) >> 4), v11);
  v13 = *(_QWORD *)(_R14 + 144);
  v27 = _R14;
  for ( i = *(_QWORD *)(_R14 + 152); i != v13; v13 += 80 )
  {
    v15 = (_QWORD *)_R13[40];
    if ( (unsigned __int64)v15 >= _R13[41] )
    {
      sub_697890(_R13 + 39, v13, &v29);
    }
    else
    {
      sub_682960(v15, v13, (unsigned __int8)v29);
      _R13[40] += 80LL;
    }
  }
  __asm { vxorps  xmm0, xmm0, xmm0 }
  __asm { vmovups [rbp+var_50], xmm0 }
  v31 = 8;
  v32 = 0;
  v33 = unk_19C8208;
  v29 = &unk_18DCC18;
  v28 = 19246190;
  v17 = sub_1AF950(&v28, 19246190);
  _RBX = _R13 + 21;
  sub_698910(_R13 + 21, v28, &v29, 0, v17);
  v29 = &unk_18DCC18;
  sub_A660(&v29, 0);
  if ( v32 == 0 )
    sub_37B800(v30, v19);
  *((_DWORD *)_R13 + 40) = 0;
  v20 = *(_QWORD *)(v27 + 144);
  *((_DWORD *)_R13 + 65) = *(_DWORD *)(v20 + 20);
  *((_DWORD *)_R13 + 68) = *(_DWORD *)(v20 + 16);
  _R13[33] = *(_QWORD *)(v20 + 8);
  _R13[35] = 0xCCCCCCCCCCCCCCCDLL * ((__int64)(*(_QWORD *)(v27 + 152) - *(_QWORD *)(v27 + 144)) >> 4);
  __asm
  {
    vmovups ymm0, ymmword ptr [rbx+70h]
    vmovups ymmword ptr [r13+80h], ymm0
    vmovups ymm0, ymmword ptr [rbx]
    vmovups ymm1, ymmword ptr [rbx+20h]
    vmovups ymm2, ymmword ptr [rbx+40h]
    vmovups ymm3, ymmword ptr [rbx+60h]
    vmovups ymmword ptr [r13+70h], ymm3
    vmovups ymmword ptr [r13+50h], ymm2
    vmovups ymmword ptr [r13+30h], ymm1
    vmovups ymmword ptr [r13+10h], ymm0
  }
  return 0x6365786562696C2FLL;
}


/* 0x6984C0 */
__int64 __fastcall render_texture_array_2d_destruct(_QWORD *a1)
{
  void (__fastcall ***v2)(_QWORD); // rbx
  void (__fastcall ***v3)(_QWORD); // r15

  *a1 = &unk_1935220;
  v2 = (void (__fastcall ***)(_QWORD))a1[39];
  v3 = (void (__fastcall ***)(_QWORD))a1[40];
  if ( v2 != v3 )
  {
    do
    {
      (**v2)(v2);
      v2 += 10;
    }
    while ( v3 != v2 );
    v2 = (void (__fastcall ***)(_QWORD))a1[39];
  }
  if ( v2 != nullptr )
    sub_252D30(a1 + 42, v2, a1[41] - (_QWORD)v2);
  return render_texture_destruct((__int64)a1);
}


/* 0x698540 */
double __fastcall render_texture_array_2d_delete(_QWORD *a1)
{
  void (__fastcall ***v2)(_QWORD); // rbx
  void (__fastcall ***v3)(_QWORD); // r15

  *a1 = &unk_1935220;
  v2 = (void (__fastcall ***)(_QWORD))a1[39];
  v3 = (void (__fastcall ***)(_QWORD))a1[40];
  if ( v2 != v3 )
  {
    do
    {
      (**v2)(v2);
      v2 += 10;
    }
    while ( v3 != v2 );
    v2 = (void (__fastcall ***)(_QWORD))a1[39];
  }
  if ( v2 != nullptr )
    sub_252D30(a1 + 42, v2, a1[41] - (_QWORD)v2);
  render_texture_destruct((__int64)a1);
  return sub_37BF50(a1);
}
