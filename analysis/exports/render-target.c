/* Generated Hex-Rays evidence for the common render-target lifecycle. */

/* 0x11B2CD0 */
void *__fastcall render_target_construct(__int64 a1, unsigned int a2, char a3)
{
  void *result; // rax
  _QWORD *v8; // r15

  _RBX = a1;
  sub_4486F0(a1);
  __asm { vxorps  xmm0, xmm0, xmm0 }
  result = &unk_19A5B28;
  *(_QWORD *)_RBX = &unk_19A5B28;
  *(_BYTE *)(_RBX + 12) = a3;
  __asm { vmovups xmmword ptr [rbx+10h], xmm0 }
  if ( a3 != 0 )
  {
    v8 = (_QWORD *)sub_37BF40(1552);
    result = sub_6B40A0(v8);
    *(_QWORD *)(_RBX + 16) = v8;
    *(_QWORD *)(_RBX + 24) = v8;
  }
  return result;
}


/* 0x11B2D40 */
void __fastcall render_target_destruct(__int64 a1, __m128 _XMM0)
{
  __int64 v3; // rdi

  *(_QWORD *)a1 = &unk_19A5B28;
  if ( *(_BYTE *)(a1 + 12) != 0 )
  {
    v3 = *(_QWORD *)(a1 + 16);
    _R14 = a1 + 16;
    if ( v3 != 0 )
      *(double *)_XMM0.m128_u64 = (*(double (__fastcall **)(__int64, double))(*(_QWORD *)v3 + 8LL))(
                                    v3,
                                    *(double *)_XMM0.m128_u64);
    __asm
    {
      vxorps  xmm0, xmm0, xmm0
      vmovups xmmword ptr [r14], xmm0
    }
  }
  nullsub_43();
}


/* 0x11B2D90 */
double __fastcall render_target_delete(__int64 a1, __m128 _XMM0)
{
  __int64 v3; // rdi

  *(_QWORD *)a1 = &unk_19A5B28;
  if ( *(_BYTE *)(a1 + 12) != 0 )
  {
    v3 = *(_QWORD *)(a1 + 16);
    _R14 = a1 + 16;
    if ( v3 != 0 )
      *(double *)_XMM0.m128_u64 = (*(double (__fastcall **)(__int64))(*(_QWORD *)v3 + 8LL))(v3);
    __asm
    {
      vxorps  xmm0, xmm0, xmm0
      vmovups xmmword ptr [r14], xmm0
    }
  }
  nullsub_43();
  return sub_37BF50(a1);
}


/* 0x11B2DE0 */
__int64 render_target_active_buffer_index()
{
  return 0;
}


/* 0x11B2DF0 */
__int64 __fastcall render_target_active_state_handle(__int64 a1)
{
  return a1 + 24;
}


/* 0x11B2E00 */
void __fastcall render_target_set_state(__int64 a1, __int64 a2)
{
  *(_QWORD *)(a1 + 16) = a2;
  *(_QWORD *)(a1 + 24) = a2;
}
