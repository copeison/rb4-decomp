/* Generated Hex-Rays evidence for the common render 1D texture-array lifecycle. */

/* 0x696C00 */
__int64 __fastcall render_texture_array_1d_construct(_QWORD *a1, __int64 a2)
{
  double v11; // xmm0_8
  __int64 v12; // rbx
  __int64 i; // r14
  _QWORD *v14; // rdi
  char *v15; // rsi
  char *v16; // rax
  unsigned __int64 v18; // rax
  _DWORD *v19; // rcx
  unsigned __int64 v20; // rdx
  unsigned __int64 v21; // r13
  __int64 v22; // r15
  __int64 v23; // rdi
  char *v24; // r14
  __int64 v25; // rax
  __int64 v26; // rax
  __int64 v33; // [rsp+8h] [rbp-48h]
  __int64 v34; // [rsp+10h] [rbp-40h]
  unsigned __int8 v35; // [rsp+1Fh] [rbp-31h] BYREF
  __int64 v36; // [rsp+20h] [rbp-30h]

  _R14 = a2;
  _R12 = a1;
  v36 = 0x6365786562696C2FLL;
  render_texture_construct(a1);
  *_R12 = &unk_19350F0;
  v35 = sub_69B990(_R14);
  __asm
  {
    vmovups ymm0, ymmword ptr [r14+70h]
    vmovups ymmword ptr [r12+118h], ymm0
    vmovups ymm0, ymmword ptr [r14]
    vmovups ymm1, ymmword ptr [r14+20h]
    vmovups ymm2, ymmword ptr [r14+40h]
    vmovups ymm3, ymmword ptr [r14+60h]
    vmovups ymmword ptr [r12+108h], ymm3
    vmovups ymmword ptr [r12+0E8h], ymm2
    vmovups ymmword ptr [r12+0C8h], ymm1
    vmovups ymmword ptr [r12+0A8h], ymm0
    vxorps  xmm0, xmm0, xmm0
    vmovups xmmword ptr [r12+138h], xmm0
  }
  _R12[41] = 0;
  v11 = nullsub_18(_R12 + 42, "EASTL vector");
  sub_697470(_R12 + 39, 0xCCCCCCCCCCCCCCCDLL * ((__int64)(*(_QWORD *)(_R14 + 152) - *(_QWORD *)(_R14 + 144)) >> 4), v11);
  v12 = *(_QWORD *)(_R14 + 144);
  v33 = _R14;
  for ( i = *(_QWORD *)(_R14 + 152); i != v12; v12 += 80 )
  {
    v14 = (_QWORD *)_R12[40];
    if ( (unsigned __int64)v14 >= _R12[41] )
    {
      sub_697890(_R12 + 39, v12, &v35);
    }
    else
    {
      sub_682960(v14, v12, v35);
      _R12[40] += 80LL;
    }
  }
  v15 = (char *)_R12[39];
  v16 = (char *)_R12[40];
  _RBX = _R12 + 21;
  if ( v15 != v16 )
  {
    v18 = 0xCCCCCCCCCCCCCCCDLL * ((v16 - v15) >> 4);
    if ( v18 <= 0x800 )
    {
      v19 = v15 + 16;
      v20 = 0;
      while ( *(v19 - 1) == 1 && *v19 == 1 )
      {
        ++v20;
        v19 += 20;
        if ( v20 >= v18 )
        {
          if ( v18 >= 2 )
          {
            v21 = 1;
            v22 = 80;
            v23 = _R12[39];
            do
            {
              if ( *(_DWORD *)(v23 + v22 + 8) != *((_DWORD *)v15 + 2) )
                break;
              if ( *(_DWORD *)(v23 + v22 + 20) != *((_DWORD *)v15 + 5) )
                break;
              v24 = v15;
              v34 = sub_6832D0(v22 + v23);
              v25 = sub_6832D0(v24);
              v15 = v24;
              if ( v34 != v25 )
                break;
              v23 = _R12[39];
              ++v21;
              v22 += 80;
            }
            while ( v21 < 0xCCCCCCCCCCCCCCCDLL * ((_R12[40] - v23) >> 4) );
          }
          break;
        }
      }
    }
  }
  v26 = *(_QWORD *)(v33 + 144);
  *((_DWORD *)_R12 + 65) = *(_DWORD *)(v26 + 20);
  *((_DWORD *)_R12 + 68) = *(_DWORD *)(v26 + 16);
  _R12[33] = *(_QWORD *)(v26 + 8);
  _R12[35] = 0xCCCCCCCCCCCCCCCDLL * ((__int64)(*(_QWORD *)(v33 + 152) - *(_QWORD *)(v33 + 144)) >> 4);
  __asm
  {
    vmovups ymm0, ymmword ptr [rbx+70h]
    vmovups ymmword ptr [r12+80h], ymm0
    vmovups ymm0, ymmword ptr [rbx]
    vmovups ymm1, ymmword ptr [rbx+20h]
    vmovups ymm2, ymmword ptr [rbx+40h]
    vmovups ymm3, ymmword ptr [rbx+60h]
    vmovups ymmword ptr [r12+70h], ymm3
    vmovups ymmword ptr [r12+50h], ymm2
    vmovups ymmword ptr [r12+30h], ymm1
    vmovups ymmword ptr [r12+10h], ymm0
  }
  return 0x6365786562696C2FLL;
}


/* 0x697020 */
__int64 __fastcall render_texture_array_1d_destruct(_QWORD *a1)
{
  void (__fastcall ***v2)(_QWORD); // rbx
  void (__fastcall ***v3)(_QWORD); // r15

  *a1 = &unk_19350F0;
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


/* 0x6970A0 */
double __fastcall render_texture_array_1d_delete(_QWORD *a1)
{
  void (__fastcall ***v2)(_QWORD); // rbx
  void (__fastcall ***v3)(_QWORD); // r15

  *a1 = &unk_19350F0;
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
