__int128 __usercall render_default_shadow_offset@<xmm0>(_QWORD *a1@<rdi>, __m128 _XMM0@<xmm0>)
{
  _DWORD *v3; // rax
  _QWORD *v4; // rcx
  __int64 v5; // r14
  _QWORD *v6; // rax
  bool v7; // zf
  __int128 result; // xmm0

  v3 = (_DWORD *)a1[66];
  if ( v3 == (_DWORD *)a1[67] )
  {
    __asm { vxorps  xmm0, xmm0, xmm0 }
  }
  else
  {
    v4 = a1 + 59;
    if ( a1[59] == 0 )
      v4 = a1;
    v5 = *v4;
    if ( *v4 != 0 )
    {
      sub_1ADEB0(*v4);
      v3 = (_DWORD *)a1[66];
    }
    v6 = (_QWORD *)(*(_QWORD *)(*(_QWORD *)(*(_QWORD *)(*(_QWORD *)(*(_QWORD *)(v5 + 48) + 176LL)
                                                      + 48LL * ((unsigned __int16)*v3 >> 12)
                                                      + 8)
                                          + 8LL * (*v3 & 0xFFF))
                              + 56LL)
                  + 8LL);
    do
    {
      v7 = *v6 == unk_1A892F8;
      v6 += 3;
    }
    while ( !v7 );
    _RAX = *(v6 - 4);
    __asm { vmovss  xmm0, dword ptr [rax+140h] }
    if ( v5 != 0 )
    {
      __asm { vmovss  [rbp+var_14], xmm0 }
      sub_1ADEF0(v5);
      __asm { vmovss  xmm0, [rbp+var_14] }
    }
  }
  return result;
}
