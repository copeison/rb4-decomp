__int64 __fastcall render_configure_default_shadowed_spot(__int64 a1, __int64 a2)
{
  __int64 v4; // rbx
  __int64 v5; // rdx
  _QWORD *v6; // rax
  __int64 v10; // r8
  __int64 v11; // r9
  __int64 v12; // rbx
  __int64 v13; // rdx
  _QWORD *v14; // rax
  __m256 v22; // [rsp+0h] [rbp-60h] BYREF
  unsigned int v23; // [rsp+20h] [rbp-40h]
  int v24; // [rsp+24h] [rbp-3Ch]
  int v25; // [rsp+28h] [rbp-38h]
  int v26; // [rsp+2Ch] [rbp-34h]
  __int64 v27; // [rsp+38h] [rbp-28h]

  _R14 = a1;
  v27 = 0x6365786562696C2FLL;
  if ( *(_DWORD *)(a2 + 64) != 0 )
  {
    v4 = 0;
    v5 = 24LL * *(unsigned int *)(a2 + 64);
    v6 = (_QWORD *)(*(_QWORD *)(a2 + 56) + 8LL);
    while ( *v6 != unk_1A892F8 )
    {
      v6 += 3;
      v5 -= 24;
      if ( v5 == 0 )
        goto LABEL_8;
    }
    v4 = *(v6 - 1);
  }
  else
  {
    v4 = 0;
  }
LABEL_8:
  __asm { vmovss  xmm0, dword ptr [r14+234h] }
  sub_49F690(v4, _XMM0);
  __asm { vmovss  xmm0, dword ptr [r14+234h] }
  __asm { vaddss  xmm0, xmm0, xmm0 }
  sub_49F720(v4, _XMM0);
  if ( *(_DWORD *)(a2 + 64) != 0 )
  {
    v12 = 0;
    v13 = 24LL * *(unsigned int *)(a2 + 64);
    v14 = (_QWORD *)(*(_QWORD *)(a2 + 56) + 8LL);
    while ( *v14 != unk_19E46E8 )
    {
      v14 += 3;
      v13 -= 24;
      if ( v13 == 0 )
        goto LABEL_15;
    }
    v12 = *(v14 - 1);
  }
  else
  {
    v12 = 0;
  }
LABEL_15:
  _RAX = &unk_19E666C;
  __asm { vmovups ymm0, ymmword ptr [rax] }
  v23 = dword_19E668C[0];
  __asm
  {
    vmovups [rbp+var_60], ymm0
    vmovss  xmm0, dword ptr [r14+234h]
    vmulss  xmm1, xmm0, dword ptr [rbp+var_60+18h]
    vmulss  xmm2, xmm0, dword ptr [rbp+var_60+1Ch]
    vmulss  xmm0, xmm0, [rbp+var_40]
    vmovss  [rbp+var_3C], xmm1
    vmovss  [rbp+var_38], xmm2
    vmovss  [rbp+var_34], xmm0
  }
  ((void (__fastcall *)(__int64, __m256 *, __int64, _QWORD, __int64, __int64))sub_127730)(
    v12 + 24,
    &v22,
    1,
    dword_19E668C[0],
    v10,
    v11);
  *(_DWORD *)(v12 + 48) = v24;
  *(_DWORD *)(v12 + 52) = v25;
  *(_DWORD *)(v12 + 56) = v26;
  *(_BYTE *)(v12 + 72) |= 1u;
  return 0x6365786562696C2FLL;
}
